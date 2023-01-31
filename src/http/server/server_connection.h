// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include <memory>
#include <map>
#include "http/common/connection.h"
#include "http/server/server.h"

namespace libevent_cpp {

// HTTP 连接处理类
class http_server_connection : public http_connection {
 public:
    http_server_connection(
        std::shared_ptr<event_base> base, int fd,
        std::shared_ptr<std::map<std::string, HandleCallBack>> handle_callbacks);
    ~http_server_connection() = default;

    int create_request();

    void fail(http_connection_error err) override;
    void do_read_done() override;
    void do_write_done() override;

 private:
    // 处理请求
    void handle_request(http_request* req);

 public:
    inline void set_client_address(const std::string& client_address) {
        client_address_ = client_address;
    }
    inline std::string get_client_address() const {
        return client_address_;
    }
    inline void set_client_port(uint16_t client_port) {
        client_port_ = client_port;
    }
    inline uint16_t get_client_port() const {
        return client_port_;
    }

 private:
    // 读超时的回调
    static void read_timeout_cb(http_server_connection* conn);
    // 写超时的回调
    static void write_timeout_cb(http_server_connection* conn);

 private:
    // 存储服务器不同路径对应的回调函数
    std::shared_ptr<std::map<std::string, HandleCallBack>> handle_callbacks_;
    // 客户端的地址
    std::string client_address_;
    // 客户端的端口
    uint16_t client_port_;
};

}  // namespace libevent_cpp

