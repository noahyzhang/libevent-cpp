// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include <memory>
#include <map>
#include "util/util_buffer.h"
#include "util/util_string.h"
#include "http/common/connection.h"

namespace libevent_cpp {

// http 请求的类别
enum http_request_kind {
    REQUEST,
    RESPONSE
};

// http 方法的类型，目前支持 get、post、head 这三种方法
enum http_method_type {
    REQUEST_GET,
    REQUEST_POST,
    REQUEST_HEAD
};

// 消息读取的状态
enum message_read_status {
    ALL_DATA_READED = 1,
    MORE_DATA_EXPECTED = 0,
    DATA_CORRUPTED = -1,
    REQUEST_CANCELED = -2
};

// 返回码
#define HTTP_OK 200
#define HTTP_BADREQUEST 400
#define HTTP_NOT_FOUND 404

// 一些标志
#define PROXY_REQUEST 0x0002

// HTTP 的请求
class http_request {
 public:
    explicit http_request(http_connection* conn);
    ~http_request() = default;

    void reset();

    // 发送 error 的回复
    void send_error_reply(int errorno, std::string reason);

    // 发送 404 的回复
    void send_not_found_reply();

    void send_reply(int code, const std::string& reason, std::unique_ptr<buffer> buf);

    // 解析 http 请求/响应的第一行
    enum message_read_status parse_first_line(const std::string& line);

    // 构建 http 请求/响应头
    void create_header();

 private:
    // 
    void send_page(std::unique_ptr<buffer> buf);

    void send_reply_start(int code, const std::string& reason);
    void send_reply_chunk(std::unique_ptr<buffer> buf);
    void send_reply_end();

    void make_header();
    void internal_send(std::unique_ptr<buffer> buf);

    // 解析 http 请求的第一行，应该是：method uri version
    int parse_request_first_line(const std::string& line);
    // 解析 http 响应的第一行，应该是：http-version status-code status-reason
    int parse_response_first_line(const std::string& line);

    // 构建 http 请求头
    void create_request_header();
    // 构建 http 响应头
    void create_response_header();

    // 设置响应的信息
    inline void set_response_info(int code, const std::string& reason) {
        kind_ = RESPONSE;
        response_status_code_ = code;
        response_status_reason_ = reason;
    }

 public:
    inline void set_http_connection(http_connection* conn) {
        conn_ = conn;
    }
    inline void set_http_request_kind(http_request_kind kind) {
        kind_ = kind;
    }
    inline void set_remote_host(const std::string& remote_host) {
        remote_host_ = remote_host;
    }
    inline void set_remote_port(const uint16_t remote_port) {
        remote_port_ = remote_port;
    }
    inline void set_cb(void (*cb)(http_request*)) {
        cb_ = cb;
    }
    inline std::function<void(http_request*)> get_cb() const {
        return cb_;
    }
    inline void set_uri(const std::string& uri) {
        uri_ = uri;
    }
    inline std::string get_uri() const {
        return uri_;
    }
    inline bool is_connection_keepalive() {
        std::string connection = input_headers_["Connection"];
        return (!connection.empty() && util_string::is_equals(connection, "keep-alive"));
    }
    inline bool is_in_connection_close() {
        if (proxy_flag_ & PROXY_REQUEST) {
            // 代理服务器
            std::string connection = input_headers_["Proxy-Connection"];
            return (connection.empty() || util_string::is_equals(connection, "keep-alive"));
        } else {
            std::string connection = input_headers_["Connection"];
            return (!connection.empty() && util_string::is_equals(connection, "close"));
        }
    }
    inline bool is_out_connection_close() {
        if (proxy_flag_ * PROXY_REQUEST) {
            // 代理服务器
            std::string connection = output_headers_["Proxy-Connection"];
            return (connection.empty() || util_string::is_equals(connection, "keep-alive"));
        } else {
            std::string connection = output_headers_["Connection"];
            return (!connection.empty() && util_string::is_equals(connection, "close"));
        }
    }
    inline bool is_connection_close() {
        return (http_minor_version_ == 0 && !is_connection_keepalive()) ||
                is_in_connection_close() ||
                is_out_connection_close();
    }
    inline bool is_handled() const {
        return is_handled_;
    }
    inline void set_handled() {
        is_handled_ = true;
    }

 private:
    http_connection* conn_;
    int proxy_flag_;
    bool is_handled_{false};

    std::unique_ptr<buffer> input_buffer_;
    std::unique_ptr<buffer> output_buffer_;

    std::function<void(http_request*)> cb_ = nullptr;

    // 对端的 host
    std::string remote_host_;
    // 对端的 port
    uint16_t remote_port_;
    // http 的种类（请求/响应）
    enum http_request_kind kind_;
    // http 方法的类型
    enum http_method_type method_type_;
    // http 的 uri
    std::string uri_;
    // http 的主版本号，例如 http1.0 中的 1
    uint16_t http_major_version_ = 1;
    // http 的副版本号，例如 http1.0 中的 0
    uint16_t http_minor_version_ = 1;
    // http 返回状态码
    int response_status_code_;
    // http 返回状态理由
    std::string response_status_reason_;

 public:
    std::map<std::string, std::string> input_headers_;
    std::map<std::string, std::string> output_headers_;
};

}  // namespace libevent_cpp
