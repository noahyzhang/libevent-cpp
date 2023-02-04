// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include "util/util_buffer.h"
#include "common.h"

namespace libevent_cpp {

class http_response {
public:
    // 设置头部
    void set_header(const std::string& key, const std::string& value);
    // 设置正文
    void set_content(const std::string& content, const std::string &content_type);

    // 获取 http 版本
    inline std::string get_http_version() const { return version_; }
    // 获取状态码
    inline int get_status_code() const { return status_code_; }
    // 获取状态的文本
    inline std::string get_status_reason() const { return status_reason_; }
    // 获取头部
    inline Headers get_headers() const { return headers_; }
    // 获取正文
    inline std::string get_body() const { return body_; }

public:
    int parse_http_response(std::shared_ptr<buffer> buffer);
    int parse_response_first_line(const std::string& data);
    int parse_response_header(const std::string& data);

public:
    // http 的版本
    std::string version_;
    // 状态码
    int status_code_;
    // 状态码的文本
    std::string status_reason_;
    // 头部
    Headers headers_;
    // 正文
    std::string body_;
};

}  // namespace libevent_cpp
