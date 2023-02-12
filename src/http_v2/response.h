// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include "util/util_buffer.h"
#include "common.h"

namespace libevent_cpp {

class http_response {
public:
    // 设置 http 版本
    void set_version(const std::string& version) {
        version_ = version;
    }
    // 设置 http 状态码
    void set_status_code(int status_code) {
        status_code_ = status_code;
    }
    // 设置 http 状态码的文本
    void set_status_reason(const std::string& status_reason) {
        status_reason_ = status_reason;
    }
    // 设置 http 响应的首行
    int set_resp_first_line(const std::string& first_line);
    // 设置头部
    void set_header(const std::string& key, const std::string& value);
    void set_header(const std::string& data);
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
    inline std::string get_header_value(const std::string& key) const {
        auto iter = headers_.find(key);
        if (iter == headers_.end()) {
            return "";
        } else {
            return iter->second;
        }
    }
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
