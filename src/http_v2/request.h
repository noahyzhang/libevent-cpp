// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include "common.h"

namespace libevent_cpp {

class Request {


private:
    // 请求方法
    std::string method_;
    // 请求的路径
    std::string path_;
    // 请求的头部
    Headers headers_;
    // 请求的主体
    std::string body_;

    // 对端的地址
    std::string remote_addr_;
    // 对端的端口
    uint16_t remote_port = -1;
    // 本地的地址
    std::string local_addr_;
    // 本地的端口
    uint16_t local_port_;


};

}  // namespace libevent_cpp
