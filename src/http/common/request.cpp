// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <utility>
#include <vector>
#include "http/common/request.h"

namespace libevent_cpp {

http_request::http_request(http_connection* conn)
    : conn_(conn) {
    kind_ = RESPONSE;  // 默认是 response
    input_buffer_ = std::unique_ptr<buffer>(new buffer());
    output_buffer_ = std::unique_ptr<buffer>(new buffer());
}

void http_request::reset() {
    input_buffer_->reset();
    output_buffer_->reset();
    uri_ = "";
    cb_ = nullptr;
    input_headers_.clear();
    output_headers_.clear();
    // TODO  
}

void http_request::send_error_reply(int errorno, std::string reason) {
    std::string err_page = "<html><head>";
    err_page += "<title>" + std::to_string(errorno) + " " + reason + "</title>\n";
    err_page += "</head><body>\n";
    err_page += "<h1>Method Not Implemented</h1>\n";
    err_page += "<p>Invalid method in request</p>\n";
    err_page += "</body></html>\n";

    auto buf = std::unique_ptr<buffer>(new buffer());
    buf->push_back_string(err_page);

    input_headers_["Connection"] = "close";  // 短连接
    set_response_info(errorno, reason);
    send_page(std::move(buf));
}

void http_request::send_not_found_reply() {
    // 对 uri 进行转义(HTML)
    std::string uri_html = util_string::convert_str_to_html(uri_);
    std::string not_found_page = "<html><head><title>404 Not Found</title></head>";
    not_found_page += "<body><h1>Not Found</h1>\n";
    not_found_page += "<p>The requested URL " + uri_html + " was not found on this server</p>";
    not_found_page += "</body></html>\n";

    auto buf = std::unique_ptr<buffer>(new buffer());
    buf->push_back_string(not_found_page);
    set_response_info(HTTP_NOT_FOUND, "Not Found");
    send_page(std::move(buf));
}

void http_request::send_page(std::unique_ptr<buffer> buf) {
    if (!http_major_version_ || !http_minor_version_) {
        http_major_version_ = http_minor_version_ = 1;
    }
    // TODO
    output_headers_.clear();
    output_headers_["Content-Type"] = "text/html; charset=utf-8";
    output_headers_["Connection"] = "close";

    internal_send(std::move(buf));
}

void http_request::send_reply(int code, const std::string& reason, std::unique_ptr<buffer> buf) {
    set_response_info(code, reason);
    internal_send(std::move(buf));
}

void http_request::internal_send(std::unique_ptr<buffer> buf) {
    output_buffer_->push_back_buffer(buf, buf->get_cur_length());
    make_header();
    conn_->start_write();
}

enum message_read_status http_request::parse_first_line(const std::string& line) {
    if (line.empty()) return MORE_DATA_EXPECTED;
    enum message_read_status status = ALL_DATA_READED;
    switch (kind_) {
    case REQUEST:
        if (parse_request_first_line(line) < 0) {
            status = DATA_CORRUPTED;
        }
        break;
    case RESPONSE:
        if (parse_response_first_line(line) < 0) {
            status = DATA_CORRUPTED;
        }
        break;
    default:
        status = DATA_CORRUPTED;
        break;
    }
    return status;
}

int http_request::parse_request_first_line(const std::string& line) {
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
        logger::error("bad http method: %s on request: %s", method, remote_host_);
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
        logger::error("bad http version: %s on request: %s", version, remote_host_);
        return -3;
    }
    // 获取 uri
    uri_ = uri;
    // TODO 判断是否为代理请求
    return 0;
}

int http_request::parse_response_first_line(const std::string& line) {
    // http 响应的第一行是状态行，应该为: http-version status-code reason-phrase CRLF
    std::vector<std::string> tokens = util_string::split_string(line, ' ');
    if (tokens.size() < 3) {
        logger::error("bad http response status line: %s", line);
        return -1;
    }
    std::string version = tokens[0];
    std::string status_code = tokens[1];
    std::string status_reason = tokens[2];
    // 获取 http 版本
    if (version == "HTTP/1.0") {
        http_major_version_ = 1;
        http_minor_version_ = 0;
    } else if (version == "HTTP/1.1") {
        http_major_version_ = 1;
        http_minor_version_ = 1;
    } else {
        logger::error("bad http response version: %s on response: %s", version, remote_host_);
        return -2;
    }
    // 获取状态码和状态理由
    response_status_code_ = std::stoi(status_code);
    response_status_reason_ = status_reason;
    return 0;
}

void http_request::create_header() {
    if (kind_ == REQUEST) {
        create_request_header();
    } else {
        create_response_header();
    }
    for (const auto& x : output_headers_) {
        conn_->write_string(x.first + ":" + x.second + "\r\n");
    }
    conn_->write_string("\r\n");
    if (output_buffer_->get_cur_length() > 0) {
        conn_->write_buffer(output_buffer_);
    }
}

void http_request::create_request_header() {
    output_headers_.erase("Proxy-Connection");
    std::string method;
    switch (method_type_) {
    case REQUEST_GET:
        method = "GET";
        break;
    case REQUEST_POST:
        method = "POSE";
        break;
    case REQUEST_HEAD:
        method = "HEAD";
        break;
    default:
        method = "";
        break;
    }
    std::string first_line = method + " " + uri_ + " HTTP/"
        + std::to_string(http_major_version_) + "." + std::to_string(http_minor_version_) + "\r\n";
    conn_->write_string(first_line);
    // 如果是 post 请求，这里尝试添加一下 content 的长度
    if (method_type_ == REQUEST_POST && output_headers_["Content-Length"].empty()) {
        std::string content_size = std::to_string(output_buffer_->get_cur_length());
        output_headers_["Content-Length"] = content_size;
    }
}

void http_request::create_response_header() {
    auto is_keepalive = is_connection_keepalive();
    std::string first_line = "HTTP/" + std::to_string(http_major_version_) + "."
        + std::to_string(http_minor_version_) + " " + std::to_string(response_status_code_)
        + " " + response_status_reason_ + "\r\n";
    conn_->write_string(first_line);
    // 针对不同 http 版本，添加不同头部
    if (http_major_version_ == 1) {
        if (http_minor_version_ == 0 && is_keepalive) {
            output_headers_["Connection"] = "keep-alive";
        }
        if (http_minor_version_ == 1 && is_keepalive) {
            output_headers_["Connection"] = "keep-alive";
            if (output_headers_["Transfer-Encoding"].empty() && output_headers_["Content-Length"].empty()) {
                output_headers_["Content-Length"] = std::to_string(output_buffer_->get_cur_length());
            }
        }
    }
    if (output_buffer_->get_cur_length() > 0 && output_headers_["Content-Type"].empty()) {
        output_headers_["Content-Type"] = "text/html; charset=utf-8";
    }
    // 如果请求已经告知关闭，则影响也应该关闭
    if (is_in_connection_close()) {
        output_headers_.erase("Connection");
        if (proxy_flag_ & PROXY_REQUEST) {
            output_headers_["Connection"] = "Close";
        }
        output_headers_.erase("Proxy-Connection");
    }
}

}  // namespace libevent_cpp
