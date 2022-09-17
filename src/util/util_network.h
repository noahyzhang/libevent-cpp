// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <netdb.h>
#include <string>
#include <memory>

namespace libevent_cpp {

class util_network {
 public:
    static int set_fd_nonblock(int fd);
    static int get_nonblock_socket();
    static int socket_connect(int fd, const std::string& address, unsigned short port);
    static int close_fd(int fd);
    static int bind_socket(const std::string& address, unsigned short port, bool reuse = true);
    static int listen_fd(int fd);
    static int accept_socket(int fd, std::shared_ptr<std::string> host, std::shared_ptr<int> port);
    static struct addrinfo* get_addr_info(const std::string& address, unsigned short port);
};

}  // namespace libevent_cpp
