// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <memory>
#include <string>
#include "base/event_base.h"
#include "http/client/client_connection.h"

namespace libevent_cpp {

class http_client {

 public:
    int timeout_ = -1;
    std::shared_ptr<event_base> base_ = nullptr;

 public:
    http_client();
    ~http_client() = default;

    inline void set_timeout(int sec) {
        timeout_ = sec;
    }
    std::unique_ptr<http_client_connection> make_connection(
        const std::string& server_address, unsigned int server_port);
    void run();
};

}  // namespace libevent_cpp
