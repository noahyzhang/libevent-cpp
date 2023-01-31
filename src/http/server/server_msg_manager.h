// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include <map>

namespace libevent_cpp {

// http 请求方法的类型，目前支持 get、post、head 这三种方法
enum http_request_method_type {
    REQUEST_GET,
    REQUEST_POST,
    REQUEST_HEAD
};

// http server 的消息解析和构造
class http_server_msg_manager {

private:
    // 解析 http 请求的第一行，应该是：method uri version
    int parse_request_first_line(const std::string& line);

    // 构建 http 响应头
    void create_response_header(
        size_t response_status_code, const std::string& response_status_reason);

private:
    // http 请求方法的类型
    enum http_request_method_type method_type_;
    // http 请求的 uri
    std::string request_uri_;
    // http 的主版本号，例如 http1.0 中的 1
    uint16_t http_major_version_ = 1;
    // http 的副版本号，例如 http1.0 中的 0
    uint16_t http_minor_version_ = 1;

    // http 响应的首行
    std::string resp_first_line_;
    // http 响应的头部
    std::map<std::string, std::string> resp_header_;
    // http 响应的主体内容
    std::string resp_body_;
};

}  // namespace libevent_cpp
