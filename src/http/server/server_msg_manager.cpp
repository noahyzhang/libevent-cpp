#include <vector>
#include "util/util_logger.h"
#include "util/util_string.h"
#include "server_msg_manager.h"

namespace libevent_cpp {

int http_server_msg_manager::parse_request_first_line(const std::string& line) {
    // http 请求的第一行应该为: method uri version
    std::vector<std::string> tokens = util_string::split_string(line, ' ');
    if (tokens.size() < 3) {
        logger::error("bad http request first line: %s", line);
        return -1;
    }
    std::string method = tokens[0];
    std::string uri = tokens[1];
    std::string version = tokens[2];
    // 获取 http 方法
    if (method == "GET") {
        method_type_ = REQUEST_GET;
    } else if (method == "POSE") {
        method_type_ = REQUEST_POST;
    } else if (method == "HEAD") {
        method_type_ = REQUEST_HEAD;
    } else {
        logger::error("bad http method: %s on request first line: %s", method, line);
        return -2;
    }
    // 获取 http 版本
    if (version == "HTTP/1.0") {
        http_major_version_ = 1;
        http_minor_version_ = 0;
    } else if (version == "HTTP/1.1") {
        http_major_version_ = 1;
        http_minor_version_ = 1;
    } else {
        logger::error("bad http version: %s on request first line: %s", version, line);
        return -3;
    }
    // 获取 uri
    request_uri_ = uri;
    // TODO 判断是否为代理请求
    return 0;
}

void http_server_msg_manager::create_response_header(
    size_t response_status_code, const std::string& response_status_reason) {

    auto is_keepalive = is_connection_keepalive();
    resp_first_line_ = "HTTP/" + std::to_string(http_major_version_) + "."
        + std::to_string(http_minor_version_) + " " + std::to_string(response_status_code)
        + " " + response_status_reason + "\r\n";
    // 针对不同 http 版本，添加不同头部
    if (http_major_version_ == 1) {
        if (http_minor_version_ == 0 && is_keepalive) {
            resp_header_["Connection"] = "keep-alive";
        }
        if (http_minor_version_ == 1 && is_keepalive) {
            resp_header_["Connection"] = "keep-alive";
            if (resp_header_["Transfer-Encoding"].empty() && resp_header_["Content-Length"].empty()) {
                resp_header_["Content-Length"] = std::to_string(output_buffer_->get_cur_length());
            }
        }
    }
    if (output_buffer_->get_cur_length() > 0 && resp_header_["Content-Type"].empty()) {
        resp_header_["Content-Type"] = "text/html; charset=utf-8";
    }
    // 如果请求已经告知关闭，则影响也应该关闭
    if (is_in_connection_close()) {
        resp_header_.erase("Connection");
        if (proxy_flag_ & PROXY_REQUEST) {
            resp_header_["Connection"] = "Close";
        }
        resp_header_.erase("Proxy-Connection");
    }
}

}  // namespace libevent_cpp
