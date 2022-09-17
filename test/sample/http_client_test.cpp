// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include "util/util_network.h"
#include "util/util_logger.h"
#include "base/epoll_base.h"
#include "buffer_event/buffer_event.h"

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8888

int http_connect(const std::string& address, uint16_t port) {
    struct addrinfo* addr_info = libevent_cpp::util_network::get_addr_info(address, port);
    if (!addr_info) {
        return -1;
    }
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        libevent_cpp::logger::error("socket failed");
        return -2;
    }
    if (connect(sock_fd, addr_info->ai_addr, addr_info->ai_addrlen) < 0) {
        libevent_cpp::logger::error("connect failed");
        return -3;
    }
    freeaddrinfo(addr_info);
    return sock_fd;
}

void http_read_cb(libevent_cpp::buffer_event* buf_ev) {
    if (buf_ev.get)
}

void http_basic_test() {
    int fd = http_connect(DEFAULT_HOST, DEFAULT_PORT);
    auto base = std::make_shared<libevent_cpp::epoll_base>();
    auto buf_ev = std::make_shared<libevent_cpp::buffer_event>(base, fd);
    buf_ev->register_read_cb()

}

int main() {
    http_basic_test();
}
