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

// 返回码
#define HTTP_OK 200
#define HTTP_BADREQUEST 400
#define HTTP_NOT_FOUND 404

// 一些标志
#define PROXY_REQUEST 0x0002

class http_request {
 public:
    explicit http_request(http_connection* conn);
    ~http_request() = default;

    void send_error(int errorno, std::string reason);
    void send_not_found();
    void reset();
    void send_reply(int code, const std::string& reason, std::unique_ptr<buffer> buf);

 private:
    void send_page(std::unique_ptr<buffer> buf);
    void send_reply_start(int code, const std::string& reason);
    void send_reply_chunk(std::unique_ptr<buffer> buf);
    void send_reply_end();

    inline void set_response(int code, const std::string& reason) {
        kind_ = RESPONSE;
        response_code_ = code;
        response_code_line_ = reason;
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
    void make_header();
    void internal_send(std::unique_ptr<buffer> buf);
    int parse_request_line(std::string line);
    int parse_response_line(std::string line);

 private:
    http_connection* conn_;
    enum http_request_kind kind_;
    std::string remote_host_;
    uint16_t remote_port_;
    int proxy_flag_;
    bool is_handled_{false};

    std::unique_ptr<buffer> input_buffer_;
    std::unique_ptr<buffer> output_buffer_;

    std::string uri_;
    uint16_t http_major_version_ = 1;  // http 的主版本号，例如 http1.0 中的 1
    uint16_t http_minor_version_ = 1;  // http 的副版本号，例如 http1.0 中的 0

    std::function<void(http_request*)> cb_ = nullptr;

    int response_code_;  // http 返回码
    std::string response_code_line_;

    std::map<std::string, std::string> input_headers_;
    std::map<std::string, std::string> output_headers_;
};

}  // namespace libevent_cpp
