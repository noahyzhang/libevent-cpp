// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <openssl/ssl.h>
#include <memory>
#include <utility>
#include <string>
#include <mutex>
#include <thread>
#include <map>
#include "http_v2/request.h"
#include "http_v2/response.h"
#include "http_v2/common/common.h"

namespace libevent_cpp {

class http_client_result {
public:
    explicit http_client_result(std::unique_ptr<http_response>&& res, Headers&& request_headers = Headers{})
        : res_(std::move(res)),
          request_headers_(std::move(request_headers)) {}

    // http_response
    // 获取 http 版本
    inline std::string get_http_version() const { res_->get_http_version(); }
    // 获取状态码
    inline int get_status_code() const { return res_->get_status_code(); }
    // 获取状态的文本
    inline std::string get_status_reason() const { return res_->get_status_reason(); }
    // 获取单个头部
    inline std::string get_single_header(const std::string& key) const {
        auto headers = res_->get_headers();
        auto iter = headers.find(key);
        if (iter != headers.end()) {
            return iter->second;
        }
        return "";
    }
    // 获取所有头部
    inline Headers get_headers() const { return res_->get_headers(); }
    // 获取正文
    inline std::string get_body() const { return res_->get_body(); }

private:
    std::unique_ptr<http_response> res_;
    Headers request_headers_;
};

class http_client_impl {
public:

    http_client_result Get(const std::string& path);
    http_client_result Get(const std::string& path, const Headers& headers);
    http_client_result Get(const std::string& path, const Headers& headers, Progress progress);

    http_client_result Head(const std::string& path);
    http_client_result Head(const std::string& path, const Headers& headers);

    http_client_result Post(const std::string& path);
    http_client_result Post(const std::string& path, const Headers& headers);

    http_client_result Put(const std::string& path);

    http_client_result Patch(const std::string& path);

    http_client_result Delete(const std::string& path);

private:
    int sync_send_internal(http_request* req, http_response* resp, HttpError* error);

    // 处理请求
    int handle_request(const http_request& req,
        http_response* resp, bool close_connection, HttpError* err);
    // // 构造请求头
    // void makeup_request_header(http_request* req, bool close_connection);
    // 将请求写入流中（相当于发送）
    int write_request_to_stream(Stream* stream,
        const http_request& req, HttpError* err);

private:
    virtual bool is_ssl() const;

private:
    void shutdown_ssl(wrap_socket&, bool);
    void shutdown_socket(wrap_socket& socket);
    void close_socket(wrap_socket& socket);

private:
    struct wrap_socket {
        int socket = -1;
        SSL* ssl = nullptr;
        bool is_open() const { return socket != -1; }
    };

    // Socket endoint information
    // const std::string host_;
    // const int port_;
    // const std::string host_and_port_;

    // Current open socket
    // 本地的 socket
    struct wrap_socket socket_;
    // 用于请求的锁，用于同步 request
    std::recursive_mutex request_mutex_;

    // 用于 socket 的锁，用于同步 socket
    mutable std::mutex socket_mutex_;
    // 标记使用 socket 的个数。被 socket_mutex 保护
    size_t socket_requests_in_flight_ = 0;
    // 存储当前 socket 是来自那个线程。被 socket_mutex 保护
    std::thread::id socket_requests_are_from_thread_ = std::thread::id();
    // 请求结束时，socket 是否关闭。被 socket_mutex 保护
    bool socket_should_be_closed_when_request_is_done_ = false;

    // Hostname-IP map
    // std::map<std::string, std::string> addr_map_;

    // 默认的 http 头部
    Headers default_headers_;

    // Settings
    // std::string client_cert_path_;
    // std::string client_key_path_;

    // 关于连接的超时
    time_t connection_timeout_sec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_SECOND;
    time_t connection_timeout_usec_ = CPPHTTPLIB_CONNECTION_TIMEOUT_USECOND;
    // 
    time_t read_timeout_sec_ = CPPHTTPLIB_READ_TIMEOUT_SECOND;
    time_t read_timeout_usec_ = CPPHTTPLIB_READ_TIMEOUT_USECOND;
    time_t write_timeout_sec_ = CPPHTTPLIB_WRITE_TIMEOUT_SECOND;
    time_t write_timeout_usec_ = CPPHTTPLIB_WRITE_TIMEOUT_USECOND;

    // std::string basic_auth_username_;
    // std::string basic_auth_password_;
    // std::string bearer_token_auth_token_;

    std::string digest_auth_username_;
    std::string digest_auth_password_;

    bool keep_alive_ = false;
    bool follow_location_ = false;

    bool url_encode_ = true;

    int address_family_ = AF_UNSPEC;
    bool tcp_nodelay_ = CPPHTTPLIB_TCP_NODELAY;
    SocketOptions socket_options_ = nullptr;

    bool compress_ = false;
    bool decompress_ = true;

    std::string interface_;

    std::string proxy_host_;
    int proxy_port_ = -1;

    // std::string proxy_basic_auth_username_;
    // std::string proxy_basic_auth_password_;
    // std::string proxy_bearer_token_auth_token_;

    std::string proxy_digest_auth_username_;
    std::string proxy_digest_auth_password_;

    std::string ca_cert_file_path_;
    std::string ca_cert_dir_path_;

    X509_STORE *ca_cert_store_ = nullptr;

    bool server_certificate_verification_ = true;
};

}  // namespace libevent_cpp
