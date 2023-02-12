// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include <memory>
#include <utility>
#include <mutex>
#include <thread>
#include <functional>
#include "base/event_base.h"
#include "common.h"
#include "response.h"
#include "request.h"
#include "connection.h"
#include "http_v2/common/client_impl.h"

namespace libevent_cpp {

class http_client : public http_client_impl {
public:
    explicit http_client(const std::string& host);
    explicit http_client(const std::string& host, uint16_t port);
    explicit http_client(const std::string& host, uint16_t port,
        const std::string& client_cert_path, const std::string& client_key_path);
    explicit http_client(const std::string& host, uint16_t port, X509* client_cert, EVP_PKEY* client_key);
    ~http_client() override;

public:
    void set_ca_cert_store(X509_STORE* ca_cert_store);
    // 设置连接超时
    void set_connection_timeout(time_t sec, time_t usec = 0);
    // 设置读超时
    void set_read_timeout(time_t sec, time_t usec = 0);
    // 设置写超时
    void set_write_timeout(time_t sec, time_t usec = 0);

// private:
//     // 异步接口，事件管理模块开始调度
//     void event_manager_dispatch();

//     bool sync_send_internal(const http_request& req, const http_response& res);


private:
    // 为了可以实现异步接口，事件管理
    std::shared_ptr<event_base> event_manager_ = nullptr;
    // http 连接
    std::shared_ptr<http_connection> http_connection_ = nullptr;

    // 远端 server 的地址信息
    std::string server_host_;
    int server_port_;
    // const std::string server_host_and_port_;

    // // 当前打开的 socket
    // struct Socket {
    //     int sock = INVALID_SOCKET;
    //     bool is_open() const { return sock != INVALID_SOCKET; }
    // };
    // Socket socket_;
    // mutable std::mutex socket_mutex_;
    // std::recursive_mutex request_mutex_;

    // size_t socket_requests_in_flight_ = 0;
    // std::thread::id socket_requests_are_from_thread_ = std::thread::id();
    // bool socket_should_be_closed_when_request_is_done_ = false;


    // Headers default_headers_;

    SSL_CTX* ctx_;
    std::mutex ctx_mutex_;
    std::once_flag initialize_cert_;
};

}  // namespace libevent_cpp
