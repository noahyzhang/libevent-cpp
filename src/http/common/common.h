// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>

namespace libevent_cpp {

// 客户端信息
struct http_client_info {
    int peer_fd_;
    std::string host_;
    int port_;
};

}  // namespace libevent_cpp
