#include <sstream>
#include "http_v2/request.h"
#include "http_v2/common/util_algorithm.h"
#include "http_v2/common/data_stream.h"

namespace libevent_cpp {

std::string http_request::get_http_request_head_message(bool close_connection) {
    auto request_first_line = makeup_request_first_line();
    makeup_request_header(close_connection);
    BufferStream buff_stream;
    // 写入 http 首行
    buff_stream.write(request_first_line.data(), request_first_line.size());
    // 写入 http 报头
    for (const auto& header : request_headers_) {
        buff_stream.write_format("%s: %s\r\n", header.first.c_str(), header.second.c_str());
    }
    static const char* br_line = "\r\n";
    buff_stream.write(br_line, sizeof(br_line));
    return buff_stream.get_buffer();
}

// 构造 http 请求首行
std::string http_request::makeup_request_first_line() {
    // HTTP 首行
    const auto& path = is_encode_url_ ? util_algorithm::encode_url(request_path_) : request_path_;
    std::stringstream ss;
    ss << request_method_.data() << " " << request_path_.data() << " HTTP/1.1\r\n";
    return ss.str();
}

// 构造 http 请求头
void http_request::makeup_request_header(bool close_connection) {
    // 如果需要关闭连接
    // Connection 头（header）决定当前的事务完成后，是否会关闭网络连接。
    // 如果该值是“keep-alive”，网络连接就是持久的，不会关闭，使得对同一个服务器的请求可以继续在该连接上完成。
    // close 表明客户端或服务器想要关闭该网络连接，这是 HTTP/1.0 请求的默认值
    if (close_connection) {
        if (!has_header("Connection")) {
            request_headers_.emplace("Connection", "Close");
        }
    }
    // Host 请求头指明了请求将要发送到的服务器主机名和端口号。
    // 如果没有包含端口号，会自动使用被请求服务的默认端口（比如 HTTPS URL 使用 443 端口，HTTP URL 使用 80 端口）。
    // 所有 HTTP/1.1 请求报文中必须包含一个Host头字段。
    // 对于缺少Host头或者含有超过一个Host头的 HTTP/1.1 请求，可能会收到400（Bad Request）状态码
    if (!has_header("Host")) {
        // 对于 https/http 使用默认端口号(443/80)
        if (is_ssl_) {
            // HTTPS
            if (remote_port_ == 443) {
                request_headers_.emplace("Host", remote_host_);
            } else {
                request_headers_.emplace("Host", remote_host_and_port_);
            }
        } else {
            // HTTP
            if (remote_port_ == 80) {
                request_headers_.emplace("Host", remote_host_);
            } else {
                request_headers_.emplace("Host", remote_host_and_port_);
            }
        }
    }
    // Accept 请求头用来告知（服务器）客户端可以处理的内容类型，这种内容类型用MIME 类型来表示
    // */* 表示任意类型的 MIME 类型
    if (!has_header("Accept")) {
        request_headers_.emplace("Accept", "*/*");
    }
    // User-Agent 首部包含了一个特征字符串
    // 用来让网络协议的对端来识别发起请求的用户代理软件的应用类型、操作系统、软件开发商以及版本号。
    if (!has_header("User-Agent")) {
        request_headers_.emplace("User-Agent", "libevent-cpp client");
    }
    // 根据 body 判断填写一些头部信息
    if (request_body_.empty()) {

    } else {  // body 不为空
        // 设置正文类型
        if (!has_header("Content-Type")) {
            request_headers_.emplace("Content-Type", "text/plain");
        }
        if (!has_header("Content-Length")) {
            auto length = std::to_string(request_body_.size());
            request_headers_.emplace("Content-Length", length);
        }
    }

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
    // Authorization 请求标头用于提供服务器验证用户代理身份的凭据，允许访问受保护的资源
    if (!basic_auth_password_.empty() || !basic_auth_username_.empty()) {
        if (!has_header("Authorization")) {
            request_headers_.emplace(make_basic_auth_header(basic_auth_username_, basic_auth_password_, false));
        }
    }
    // Proxy-Authorization 是一个请求首部，其中包含了用户代理提供给代理服务器的用于身份验证的凭证
    // 这个首部通常是在服务器返回了 407 Proxy Authentication Required 响应状态码及 Proxy-Authenticate 首部后发送的
    if (!proxy_basic_auth_username_.empty() && !proxy_basic_auth_password_.empty()) {
        if (!has_header("Proxy-Authorization")) {
            request_headers_.insert(make_basic_auth_header(proxy_basic_auth_username_,
                proxy_basic_auth_password_, true));
        }
    }
    if (!bearer_token_auth_token_.empty()) {
        if (!has_header("Authorization")) {
            request_headers_.insert(make_bearer_auth_header(bearer_token_auth_token_, false));
        }
    }
    if (!proxy_bearer_token_auth_token_.empty()) {
        if (!has_header("Proxy-Authorization")) {
            request_headers_.insert(make_bearer_auth_header(proxy_bearer_token_auth_token_, true));
        }
    }
}

}  // namespace libevent_cpp
