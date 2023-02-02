// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include 
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
    inline void set_request_body(const std::string& body) {
        body_ = body;
    }
    inline void set_client_result_cb(http_client_result_cb cb) {
        client_result_cb_ = cb;
    }


private:
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

    // http 客户端请求答复的回调函数
    http_client_result_cb client_result_cb_ = nullptr;
};

}  // namespace libevent_cpp
