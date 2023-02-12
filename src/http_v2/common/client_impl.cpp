#include <regex>
#include "http_v2/common/util_network.h"
#include "http_v2/common/util_algorithm.h"
#include "http_v2/common/client_impl.h"

namespace libevent_cpp {

http_client_result http_client_impl::Get(const std::string& path) {
    return Get(path, Headers());
}

http_client_result http_client_impl::Get(const std::string& path, const Headers& headers) {
    // 构造 http 请求
    auto req = std::make_shared<http_request>();
    req->set_request_method("GET");
    req->set_request_path(path);
    req->set_request_headers(headers);
    return sync_send(req);
}

http_client_result http_client_impl::sync_send(std::shared_ptr<http_request> req) {
    // 构造 http 响应
    auto resp = std::make_shared<http_response>();
    auto err = std::make_shared<HttpError>();
    *err = HttpError::Success;
    auto res = sync_send_internal(req, resp, err);
    return http_client_result{resp, *err};
}

int http_client_impl::sync_send_internal(std::shared_ptr<http_request> req,
    std::shared_ptr<http_response> resp, std::shared_ptr<HttpError> err) {
    // 处理的是同步的请求，因此加锁保证请求是同步的
    std::lock_guard<std::recursive_mutex> request_mutex_guard(request_mutex_);
    {
        // 保证 socket 的使用是同步的
        std::lock_guard<std::mutex> guard(socket_mutex_);
        // 这里立刻将其设置为 false，如果他在请求结束时它被设置为 true，就说明另外一个线程希望关闭此套接字
        socket_should_be_closed_when_request_is_done_ = false;
        auto is_alive = false;
        if (socket_.is_open()) {
            // 检查 socket 是否存活
            is_alive = util_network::check_socket_alive(socket_.socket);
            if (!is_alive) {
                // 这里发现连接已经被关闭，防止 sigpipe 信号。此时直接释放资源
                // 不可能是其他线程的 request，因为我们在 request_mutex_ 锁内
                shutdown_ssl(socket_, false);
                shutdown_socket(socket_);
                close_socket(socket_);
            }
        }
        // 这里，socket 不是存活的，那应该创建一个 socket
        if (!is_alive) {
            if (!create_and_connect_socket(socket_, err)) {
                return false;
            }
            // 如果是 ssl 连接
            if (is_ssl()) {
                // TODO 这里处理 ssl 连接
            }
        }
        // 将此套接字标记为正在使用，以便此请求正在进行时，任何人都不能关闭它，即使我们将释放互斥锁
        if (socket_requests_in_flight_ > 1) {
            socket_requests_are_from_thread_ = std::this_thread::get_id();
        }
        socket_requests_in_flight_++;
        socket_requests_are_from_thread_ = std::this_thread::get_id();
    }
    // 在请求头中插入默认的头部
    for (const auto& header : default_headers_) {
        if (req->request_headers_.find(header.first) == req->request_headers_.end()) {
            req->request_headers_.insert(header);
        }
    }
    // 处理请求
    // 如果不是长连接，则需要关闭此连接
    auto close_connection = !keep_alive_;
    int res = handle_request(req, resp, close_connection, err);
    // 请求结束，进行标记
    {
        std::lock_guard<std::mutex> guard(socket_mutex_);
        socket_requests_in_flight_ -= 1;
        if (socket_requests_in_flight_ <= 0) {
            socket_requests_are_from_thread_ = std::thread::id();
        }
        if (socket_should_be_closed_when_request_is_done_ || close_connection || res < 0) {
            shutdown_ssl(socket_, true);
            shutdown_socket(socket_);
            close_socket(socket_);
        }
    }
    // 返回
    if (!res) {
        if (*err == HttpError::Success) {
            *err = HttpError::Unknown;
        }
    }
    return res;
}

int http_client_impl::handle_request(std::shared_ptr<http_request> req,
    std::shared_ptr<http_response> resp, bool close_connection, std::shared_ptr<HttpError> err) {
    // 数据流
    auto stream = std::make_shared<SocketStream>(socket_.socket, read_timeout_sec_,
        read_timeout_usec_, write_timeout_sec_, write_timeout_usec_);
    // 请求路径为空，直接返回
    if (req->request_path_.empty()) {
        *err = HttpError::Connection;
        return -1;
    }
    // 这里拷贝一份 http_request，用于后面判断是否需要做重定向等其他逻辑
    http_request req_save = *req;
    int res = 0;
    // 不是 ssl 连接，并且代理不为空
    // if (!is_ssl() && !proxy_host_.empty() && proxy_port_ != -1) {
    //     http_request req2 = *req;
    //     req2.request_path_ = "http://" + remote_host_and_port_ + req->request_path_;
    //     res = process_request(stream, req2, resp, close_connection, err);
    //     req = req2;
    //     req->request_path_ = req_save.request_path_;
    // } else {
    //     // TODO
    //     res = process_request(stream, req, resp, close_connection, err);
    // }
    res = process_request(stream, req, resp, close_connection, err);
    if (res < 0) {
        return -2;
    }
    // 3** 重定向，需要进一步的操作以完成请求
    if (resp->status_code_ > 300 && resp->status_code_ < 400 && follow_location_) {
        *req = req_save;
        res = http_redirect(req, resp, err);
    }
    if ((resp->status_code_ == 401 || resp->status_code_ == 407) && req->authorization_count_ < 5) {
        auto is_proxy = resp->status_code_ == 407;
        const auto& username = is_proxy ? proxy_digest_auth_username_ : digest_auth_username_;
        const auto& password = is_proxy ? proxy_digest_auth_password_ : digest_auth_password_;
        if (!username.empty() && !password.empty()) {
            std::map<std::string, std::string> auth;
            if (parse_www_authenticate(resp, auth, is_proxy)) {
                auto new_req = std::make_shared<http_request>();
                *new_req = *req;
                new_req->authorization_count_ += 1;
                new_req->request_headers_.erase(is_proxy ? "Proxy-Authorization" : "Authorization");
                new_req->request_headers_.insert(make_digest_authentication_header(
                    req, auth, new_req.authorization_count_, random_string(10), username, password, is_proxy));
                auto new_resp = std::make_shared<http_response>();
                res = sync_send_internal(new_req, new_resp, err);
                if (res) {
                    resp = new_resp;
                }
            }
        }
    }
    return res;
}

int http_client_impl::process_request(std::shared_ptr<Stream> stream, std::shared_ptr<http_request> req,
    std::shared_ptr<http_response> resp, bool close_connection, std::shared_ptr<HttpError> err) {
    // 发送 http 数据
    auto http_head_message = req->get_http_request_head_message(close_connection);
    if (write_request_data_to_stream(stream, http_head_message.data(), http_head_message.size()) < 0) {
        *err = HttpError::Write;
        return -1;
    }
    auto http_body_message = req->get_http_request_body_message();
    if (write_request_data_to_stream(stream, http_body_message.data(), http_body_message.size()) < 0) {
        *err = HttpError::Write;
        return -2;
    }
    // 接收 http 回包数据
    // 获取 http 回包首行
    auto resp_line = read_response_line_data_from_stream(stream);
    if (resp_line.empty()) {
        *err = HttpError::Read;
        return -3;
    }
    if (resp->set_resp_first_line(resp_line) < 0) {
        *err = HttpError::Read;
        return -4;
    }
    // 对于状态为 "100 continue" 的回包，直接忽略，直到获取到对用户有意义的回包
    // 100 Continue 继续。客户端应继续其请求
    while (resp->get_status_code() == 100) {
        // CRLF
        if (read_response_line_data_from_stream(stream).empty()) {
            *err = HttpError::Read;
            return -5;
        }
        // 响应的下一行
        resp_line = read_response_line_data_from_stream(stream);
        if (resp_line.empty()) {
            *err = HttpError::Read;
            return -6;
        }
        if (resp->set_resp_first_line(resp_line) < 0) {
            *err = HttpError::Read;
            return -7;
        }
    }
    // 获取 http 回包头部
    for (;;) {
        resp_line = read_response_line_data_from_stream(stream);
        if (resp_line.empty()) {
            *err = HttpError::Read;
            return -8;
        }
        // 检查行的末尾是否为 CRLF
        auto line_size = resp_line.size();
        if (line_size >= 2 && resp_line[line_size-2] == '\r', resp_line[line_size-1] == '\n') {
            if (line_size == 2) {
                break;
            } else {
                // 跳过非法的行
                continue;
            }
        }
        resp->set_header(resp_line);
    }
    // 获取 http 回包正文
    // 204 No Content 无内容。服务器成功处理，但未返回内容。在未更新网页的情况下，可确保浏览器继续显示当前文档
    // HEAD 类似于 GET 请求，只不过返回的响应中没有具体的内容，用于获取报头
    // CONNECT HTTP/1.1 协议中预留给能够将连接改为管道方式的代理服务器
    if ((resp->get_status_code() != 204) && (req->request_method_ != "HEAD") && (req->request_method_ != "CONNECT")) {
        auto redirect = (resp->get_status_code() > 300) && (resp->get_status_code() < 400) && follow_location_;
        // TODO req.response_handler
        // TODO req.progress

    }
    // 如果对端需要关闭连接
    if (resp->get_header_value("Connection") == "close"
        || (resp->get_http_version() == "HTTP/1.0" && resp->get_status_reason() != "Connection established")) {
        std::lock_guard<std::mutex> guard(socket_mutex_);
        shutdown_ssl(socket_, true);
        shutdown_socket(socket_);
        close_socket(socket_);
    }
    return 0;
}

int http_client_impl::http_redirect(std::shared_ptr<http_request> req,
    std::shared_ptr<http_response> resp, std::shared_ptr<HttpError> err) {
    // 检查是否允许重定向
    if (req->redirect_max_count_ == 0) {
        *err = HttpError::ExceedRedirectCount;
        return -1;
    }
    auto location = 
}

// void http_client_impl::makeup_request_header(std::shared_ptr<http_request> req, bool close_connection) {
//     // 如果需要关闭连接
//     if (close_connection) {
//         if (!req->has_header("Connection")) {
//             req->headers_.emplace("Connection", "Close");
//         }
//     }
//     if (!req->has_header("Host")) {
//         if (is_ssl()) {
//             // HTTPS
//             if (port_ == 443) {
//                 req->headers_.emplace("Host", host_);
//             } else {
//                 req->headers_.emplace("Host", host_and_port_);
//             }
//         } else {
//             // HTTP
//             if (port_ == 80) {
//                 req->headers_.emplace("Host", host_);
//             } else {
//                 req->headers_.emplace("Host", host_and_port_);
//             }
//         }
//     }
//     if (!req->has_header("Accept")) {
//         req->headers_.emplace("Accept", "*/*");
//     }
//     if (!req->has_header("User-Agent")) {
//         req->headers_.emplace("User-Agent", "libevent-cpp client");
//     }
//     // 根据 body 判断填写一些头部信息
//     if (req->body_.empty()) {
//     } else {  // body 不为空
//         // 设置正文类型
//         if (!req->has_header("Content-Type")) {
//             req->headers_.emplace("Content-Type", "text/plain");
//         }
//         if (!req->has_header("Content-Length")) {
//             auto length = std::to_string(req->body_.size());
//             req->headers_.emplace("Content-Length", length);
//         }
//     }
//     // 鉴权
//     auto make_basic_auth_header = [](const std::string& username, const std::string& password, bool is_proxy) {
//         auto field = "Basic " + util_algorithm::base64_encode(username + ":" + password);
//         auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
//         return std::make_pair(key, std::move(field));
//     };
//     auto make_bearer_auth_header = [](const std::string& token, bool is_proxy = false) {
//         auto field = "Bearer " + token;
//         auto key = is_proxy ? "Proxy-Authorization" : "Authorization";
//         return std::make_pair(key, std::move(field));
//     };
//     if (!basic_auth_password_.empty() || !basic_auth_username_.empty()) {
//         if (!req->has_header("Authorization")) {
//             req->headers_.emplace(make_basic_auth_header(basic_auth_username_, basic_auth_password_, false));
//         }
//     }
//     if (!proxy_basic_auth_username_.empty() && !proxy_basic_auth_password_.empty()) {
//         if (!req->has_header("Proxy-Authorization")) {
//             req->headers_.insert(make_basic_auth_header(proxy_basic_auth_username_, proxy_basic_auth_password_, true));
//         }
//     }
//     if (!bearer_token_auth_token_.empty()) {
//         if (!req->has_header("Authorization")) {
//             req->headers_.insert(make_bearer_auth_header(bearer_token_auth_token_, false));
//         }
//     }
//     if (!proxy_bearer_token_auth_token_.empty()) {
//         if (!req->has_header("Proxy-Authorization")) {
//             req->headers_.insert(make_bearer_auth_header(proxy_bearer_token_auth_token_, true));
//         }
//     }
// }

// int http_client_impl::write_request_to_stream(
//     std::shared_ptr<Stream> stream, const http_request& req, std::shared_ptr<HttpError> err) {
//     BufferStream bstream;
//     // HTTP 首行
//     const auto& path = url_encode_ ? util_algorithm::encode_url(req.path_) : req.path_;
//     bstream.write_format("%s %s HTTP/1.1\r\n", req.method_.c_str(), req.method_.c_str());
//     // HTTP 头部
//     for (const auto& header : req.headers_) {
//         bstream.write_format("%s: %s\r\n", header.first.c_str(), header.second.c_str());
//     }
//     const char* br_line = "\r\n";
//     bstream.write(br_line, sizeof(br_line));
//     // HTTP 正文
//     if (req.body_.empty()) {
//         return write_content_with_provider(stream, req, err);
//     }
//     if (!write_request_data_to_stream(stream, req.body_.data(), req.body_.size())) {
//         *err = HttpError::Write;
//         return -1;
//     }
//     return 0;
// }


}  // namespace libevent_cpp
