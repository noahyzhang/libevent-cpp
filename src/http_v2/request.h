// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include <utility>
#include <algorithm>
#include "common.h"

namespace libevent_cpp {

class http_request {
public:

    inline void set_request_method(const std::string& method) {
        method_ = method;
    }
    inline void set_request_path(const std::string& path) {
        path_ = path;
    }
    inline void set_request_header(const std::string& key, const std::string& value) {
        headers_.emplace(key, value);
    }
    inline void set_request_headers(const Headers& headers) {
        std::for_each(headers.begin(), headers.end(), [&](const std::string& key, const std::string& value) {
            headers_.emplace(key, value);
        });
    }
    inline void set_request_body(const std::string& body) {
        body_ = body;
    }
    inline void set_request_progress(Progress&& progress) {
        progress_ = std::move(progress);
    }
    // 用于异步调用
    inline void set_client_result_cb(http_client_result_cb cb) {
        client_result_cb_ = cb;
    }

    inline bool has_header(const std::string& key) {
        return headers_.find(key) != headers_.end();
    }

public:
    // 请求方法
    std::string method_;
    // 请求的路径
    std::string path_;
    // 请求的头部
    Headers headers_;
    // 请求的主体
    std::string body_;

    // 对端的地址
    std::string remote_addr_;
    // 对端的端口
    uint16_t remote_port = -1;
    // 本地的地址
    std::string local_addr_;
    // 本地的端口
    uint16_t local_port_;

    Progress progress_;

    // http 客户端请求答复的回调函数
    http_client_result_cb client_result_cb_ = nullptr;


    size_t redirect_count_ = CPPHTTPLIB_REDIRECT_MAX_COUNT;
    size_t content_length_ = 0;
    ContentProvider content_provider_;
    bool is_chunked_content_provider_ = false;
    size_t authorization_count_ = 0;
};

}  // namespace libevent_cpp
