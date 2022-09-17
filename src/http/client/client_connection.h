// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include <memory>
#include "http/common/connection.h"
#include "http/common/request.h"
#include "http/client/client.h"

namespace libevent_cpp {

class http_client_connection;
static void read_timeout_cb(http_client_connection* conn);
static void write_timeout_cb(http_client_connection* conn);

class http_client_connection : public http_connection {
 public:
    http_client_connection(std::shared_ptr<event_base> base, int fd, int timeout);
    ~http_client_connection() = default;

    int make_request(std::unique_ptr<http_request> req);
    int connect();

    void do_read_done() override;
    void do_write_done() override;
    void fail(http_connection_error err) override;

 public:
    inline void set_server_addr(const std::string& server_addr) {
        server_addr_ = server_addr;
    }
    inline void set_server_port(unsigned int server_port) {
        server_port_ = server_port;
    }

 private:
    std::string server_addr_;
    unsigned int server_port_;
    unsigned int retry_cnt_ = 0;
    unsigned int retry_max_ = 0;
};

}  // namespace libevent_cpp
