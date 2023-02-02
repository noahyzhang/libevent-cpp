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

namespace libevent_cpp {

class http_client_result {
public:
    explicit http_client_result(std::unique_ptr<http_response>&& res, Headers&& request_headers = Headers{})
        : res_(std::move(res)),
          request_headers_(std::move(request_headers)) {}

    // http_response
    int get_status_code() const { return res_->get_status_code(); }



private:
    std::unique_ptr<http_response> res_;
    Headers request_headers_;
};

class http_client {
public:
    http_client(const std::string& host, uint16_t port);

public:
    http_client_result Get(const std::string& path);
    http_client_result Get(const std::string& path, const Headers& headers);
    void GetAsync(const std::string& path, http_client_result_cb cb);

    http_client_result Head(const std::string& path);
    http_client_result Head(const std::string& path, const Headers& headers);

    http_client_result Post(const std::string& path);
    http_client_result Post(const std::string& path, const Headers& headers);

    http_client_result Put(const std::string& path);

    http_client_result Patch(const std::string& path);

    http_client_result Delete(const std::string& path);

private:
    // 异步接口，事件管理模块开始调度
    void event_manager_dispatch();

    bool send_internal(http_request& req, http_response& res);


private:
    // 为了可以实现异步接口，事件管理
    std::shared_ptr<event_base> event_manager_ = nullptr;
    // http 连接
    std::shared_ptr<http_connection> http_connection_ = nullptr;

    // 远端 server 的地址信息
    const std::string server_host_;
    const int server_port_;
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

};

}  // namespace libevent_cpp
