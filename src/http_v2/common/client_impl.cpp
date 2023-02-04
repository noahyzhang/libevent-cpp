#include "http_v2/common/util_network.h"
#include "http_v2/common/util_algorithm.h"
#include "http_v2/common/data_stream.h"
#include "http_v2/common/client_impl.h"

namespace libevent_cpp {

int http_client_impl::sync_send_internal(http_request* req, http_response* resp, HttpError* err) {
    // 处理的是同步的请求，因此加锁保证请求是同步的
    std::lock_guard<std::recursive_mutex> request_mutex_guard(request_mutex_);
    {
        // 保证 socket 的使用是同步的
        std::lock_guard<std::mutex> guard(socket_mutex_);
        socket_should_be_closed_when_request_is_done_ = false;
        auto is_alive = false;
        if (socket_.is_open()) {
            is_alive = util_network::check_socket_alive(socket_.socket);
            if (!is_alive) {
                // 这里发现连接已经被关闭，防止 sigpipe 信号。此时直接释放资源
                shutdown_ssl(socket_, false);
                shutdown_socket(socket_);
                close_socket(socket_);
            }
        }
        // 这里，socket 不是打开的，那应该打开 socket
        if (!is_alive) {
            if (!create_and_connect_socket(socket_, err)) {
                return false;
            }
            // 如果是 ssl 连接
            if (is_ssl()) {

            }
        }

        if (socket_requests_in_flight_ > 1) {

        }
    }

    // 在请求头中插入默认的头部
    for (const auto& header : default_headers_) {
        if (req->headers_.find(header.first) == req->headers_.end()) {
            req->headers_.insert(header);
        }
    }
    // 处理请求
    auto stream = std::make_shared<SocketStream>(
        socket_.socket, read_timeout_sec_, read_timeout_usec_,
        write_timeout_sec_, write_timeout_usec_);
    process_request(stream.get(), *req, resp, close_connection, err);

    // 请求结束，进行标记
    {
        std::lock_guard<std::mutex> guard(socket_mutex_);
        socket_requests_in_flight_ -= 1;
        if (socket_requests_in_flight_ <= 0) {
            socket_requests_are_from_thread_ = std::thread::id();
        }
        if (socket_should_be_closed_when_request_is_done_ || close_connection || !res) {
            shutdown_ssl(socket_, true);
            shutdown_socket(socket_);
            close_socket(socket_);
        }
    }
    // 返回
    if (!res) {
        if (err == HttpError::Success) {
            err = HttpError::Unknown;
        }
    }
    return res;
}

int http_client_impl::process_request(Stream* stream, const http_request& req,
    http_response* resp, bool close_connection, HttpError* err) {
    if (req.path_.empty()) {
        *err = HttpError::Connection;
        return -1;
    }
    auto req_save = req;
    bool res = false;
    // 不是 ssl 连接，并且代理不为空
    if (!is_ssl() && !proxy_host_.empty() && proxy_port_ != -1) {
        auto req2 = req;
        req2.path_ = "http://" + host_and_port_ + req.path_;
    } else {
        // 
    }
    if (!res) {
        return -2;
    }
    if (resp->status_code_ > 300 && resp->status_code_ < 400 && follow_location_) {
        req = req_save;
        res = redirect();
    }
    if ((resp->status_code_ == 401 || resp->status_code_ == 407) && req.authorization_count_ < 5) {
        auto is_proxy = res->status_code == 407;
        const auto& username = is_proxy ? proxy_dige
    }

}

void http_client_impl::makeup_request_header(std::shared_ptr<http_request> req, bool close_connection) {
    // 如果需要关闭连接
    if (close_connection) {
        if (!req->has_header("Connection")) {
            req->headers_.emplace("Connection", "Close");
        }
    }
    if (!req->has_header("Host")) {
        if (is_ssl()) {
            // HTTPS
            if (port_ == 443) {
                req->headers_.emplace("Host", host_);
            } else {
                req->headers_.emplace("Host", host_and_port_);
            }
        } else {
            // HTTP
            if (port_ == 80) {
                req->headers_.emplace("Host", host_);
            } else {
                req->headers_.emplace("Host", host_and_port_);
            }
        }
    }
    if (!req->has_header("Accept")) {
        req->headers_.emplace("Accept", "*/*");
    }
    if (!req->has_header("User-Agent")) {
        req->headers_.emplace("User-Agent", "libevent-cpp client");
    }
    // 根据 body 判断填写一些头部信息
    if (req->body_.empty()) {
    } else {  // body 不为空
        // 设置正文类型
        if (!req->has_header("Content-Type")) {
            req->headers_.emplace("Content-Type", "text/plain");
        }
        if (!req->has_header("Content-Length")) {
            auto length = std::to_string(req->body_.size());
            req->headers_.emplace("Content-Length", length);
        }
    }
    // 鉴权
    auto make_basic_auth_header = [](const std::string& username, const std::string& password, bool is_proxy) {
        auto field = "Basic " + util_algorithm::base64_encode(username + ":" + password);
        auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
        return std::make_pair(key, std::move(field));
    };
    auto make_bearer_auth_header = [](const std::string& token, bool is_proxy = false) {
        auto field = "Bearer " + token;
        auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
        return std::make_pair(key, std::move(field));
    };
    if (!basic_auth_password_.empty() || !basic_auth_username_.empty()) {
        if (!req->has_header("Authorization")) {
            req->headers_.emplace(make_basic_auth_header(basic_auth_username_, basic_auth_password_, false));
        }
    }
    if (!proxy_basic_auth_username_.empty() && !proxy_basic_auth_password_.empty()) {
        if (!req->has_header("Proxy-Authorization")) {
            req->headers_.insert(make_basic_auth_header(proxy_basic_auth_username_, proxy_basic_auth_password_, true));
        }
    }
    if (!bearer_token_auth_token_.empty()) {
        if (!req->has_header("Authorization")) {
            req->headers_.insert(make_bearer_auth_header(bearer_token_auth_token_, false));
        }
    }
    if (!proxy_bearer_token_auth_token_.empty()) {
        if (!req->has_header("Proxy-Authorization")) {
            req->headers_.insert(make_bearer_auth_header(proxy_bearer_token_auth_token_, true));
        }
    }
}

int http_client_impl::write_request_to_stream(
    std::shared_ptr<Stream> stream, const http_request& req, std::shared_ptr<HttpError> err) {
    BufferStream bstream;
    // HTTP 首行
    const auto& path = url_encode_ ? util_algorithm::encode_url(req.path_) : req.path_;
    bstream.write_format("%s %s HTTP/1.1\r\n", req.method_.c_str(), req.method_.c_str());
    // HTTP 头部
    for (const auto& header : req.headers_) {
        bstream.write_format("%s: %s\r\n", header.first.c_str(), header.second.c_str());
    }
    const char* br_line = "\r\n";
    bstream.write(br_line, sizeof(br_line));
    // HTTP 正文
    if (req.body_.empty()) {
        return write_content_with_provider(stream, req, err);
    }
    if (!write_request_data_to_stream(stream, req.body_.data(), req.body_.size())) {
        *err = HttpError::Write;
        return -1;
    }
    return 0;
}

}  // namespace libevent_cpp
