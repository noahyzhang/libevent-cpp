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
    inline void set_remote_host_port(const std::string& host, uint16_t port) {
        remote_host_ = host;
        remote_port_ = port;
        remote_host_and_port_ = host;
        remote_host_and_port_ += std::to_string(port);
    }
    inline void set_request_method(const std::string& method) {
        request_method_ = method;
    }
    inline void set_request_path(const std::string& path) {
        request_path_ = path;
    }
    inline void set_request_header(const std::string& key, const std::string& value) {
        request_headers_.emplace(key, value);
    }
    inline void set_request_headers(const Headers& headers) {
        std::for_each(headers.begin(), headers.end(), [&](const std::string& key, const std::string& value) {
            request_headers_.emplace(key, value);
        });
    }
    inline void set_request_body(const std::string& body) {
        request_body_ = body;
    }
    inline void set_request_progress(Progress&& progress) {
        progress_ = std::move(progress);
    }
    // 用于异步调用
    inline void set_client_result_cb(http_client_result_cb cb) {
        client_result_cb_ = cb;
    }
    inline bool has_header(const std::string& key) {
        return request_headers_.find(key) != request_headers_.end();
    }
    inline void set_is_encode_url(bool is_encode_url) {
        is_encode_url_ = is_encode_url;
    }
    inline void set_is_ssl(bool is_ssl) {
        is_ssl_ = is_ssl;
    }
    inline void set_basic_auth(const std::string& basic_auth_username,
        const std::string& basic_auth_password, const std::string& bearer_token_auth_token) {
        basic_auth_username_ = basic_auth_username;
        basic_auth_password_ = basic_auth_password;
        bearer_token_auth_token_ = bearer_token_auth_token;
    }
    inline void set_proxy_basic_auth(const std::string& proxy_basic_auth_username,
        const std::string& proxy_basic_auth_password, const std::string& proxy_bearer_token_auth_token) {
        proxy_basic_auth_username_ = proxy_basic_auth_username;
        proxy_basic_auth_password_ = proxy_basic_auth_password;
        proxy_bearer_token_auth_token_ = proxy_bearer_token_auth_token;
    }

public:
    // 获取 http 请求的头部信息
    std::string get_http_request_head_message(bool close_connection);
    std::string get_http_request_body_message();
private:
    // 构造 http 请求首行
    std::string makeup_request_first_line();
    // 构造 http 请求头
    void makeup_request_header(bool close_connection);


public:
    // 请求方法
    std::string request_method_;
    // 请求的路径
    std::string request_path_;
    // 请求的头部
    Headers request_headers_;
    // 请求的主体
    std::string request_body_;

    // 对端的主机
    std::string remote_host_;
    // 对端的端口
    uint16_t remote_port_ = -1;
    // 对端的主机+端口
    std::string remote_host_and_port_;

    // 本地的地址
    // std::string local_host_;
    // 本地的端口
    // uint16_t local_port_;

    // 鉴权的用户名
    std::string basic_auth_username_;
    // 鉴权的密码
    std::string basic_auth_password_;
    std::string bearer_token_auth_token_;
    // 代理鉴权的用户名
    std::string proxy_basic_auth_username_;
    // 代理鉴权的密码
    std::string proxy_basic_auth_password_;
    std::string proxy_bearer_token_auth_token_;

    // 是否对 URL 编码
    bool is_encode_url_ = true;
    // 是否为 ssl
    bool is_ssl_ = false;

    Progress progress_;

    // http 客户端请求答复的回调函数
    http_client_result_cb client_result_cb_ = nullptr;


    // size_t redirect_count_ = CPPHTTPLIB_REDIRECT_MAX_COUNT;
    // size_t content_length_ = 0;
    // ContentProvider content_provider_;
    // bool is_chunked_content_provider_ = false;
    // size_t authorization_count_ = 0;
};

}  // namespace libevent_cpp
