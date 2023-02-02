// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <memory>
#include <string>
#include <queue>
#include "base/event_base.h"
#include "util/util_buffer.h"
#include "request.h"

namespace libevent_cpp {

class http_connection {
public:
    http_connection(std::shared_ptr<event_base> event_manager, const std::string& host, uint16_t port);

private:
    int create_connection(const std::string& host, uint16_t port);

public:
    int create_request(std::unique_ptr<http_request> req);

    // 读取 http 响应的数据
    int read_http_response_data(std::shared_ptr<http_response> resp);

private:
    // 获取非阻塞的 socket fd
    int get_nonblock_socket_fd();
    // 设置 socket fd 为非阻塞
    int set_socket_fd_nonblock(int fd);

private:
    int fd = -1;
    std::shared_ptr<event_base> event_manager_ = nullptr;
    // 存储用户的请求
    std::queue<std::unique_ptr<http_request>> request_qu;
    std::shared_ptr<buffer> buffer_;
};

}  // namespace libevent_cpp
