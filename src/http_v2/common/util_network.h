// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <time.h>
#include <sys/types.h>
#include <memory>
#include <string>

namespace libevent_cpp {

class util_network {
public:
    static ssize_t check_socket_readable_by_select(int socket, time_t read_timeout_sec, time_t read_timeout_usec);
    static ssize_t check_socket_writeable_by_select(int socket, time_t write_timeout_sec, time_t write_timeout_usec);
    static bool check_socket_alive(int socket);
    static ssize_t recv_socket(int socket, void* buff, size_t size, int flags);
    static ssize_t send_socket(int socket, const void* buff, size_t size, int flags);
    static void get_remote_ip_and_port(int socket, std::shared_ptr<std::string> ip, std::shared_ptr<uint16_t> port);

private:
    // 处理 socket 会出现的 EINTR 错误
    template <typename T>
    static ssize_t handle_socket_EINTR(T fn);
    // 
    static bool get_ip_and_port(
        const struct sockaddr_storage& addr, socklen_t addr_len,
        std::shared_ptr<std::string> ip, std::shared_ptr<uint16_t> port);
};

}  // namespace libevent_cpp
