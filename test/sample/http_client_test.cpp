// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include "util/util_network.h"
#include "util/util_logger.h"
#include "base/epoll_base.h"
#include "http/common/request.h"
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
    if (buf_ev->get_input_buf()->find_string("This is funny") != nullptr) {
        auto req = std::make_shared<libevent_cpp::http_request>(nullptr);
        req->set_http_request_kind(libevent_cpp::RESPONSE);

        auto line = buf_ev->get_input_buf()->readline();
        req->parse_request_first_line
    }
}

void http_write_cb(libevent_cpp::buffer_event* buf_ev) {

}

void http_err_cb(libevent_cpp::buffer_event* buf_ev) {

}

void http_basic_test() {
    int fd = http_connect(DEFAULT_HOST, DEFAULT_PORT);
    auto base = std::make_shared<libevent_cpp::epoll_base>();
    auto buf_ev = std::make_shared<libevent_cpp::buffer_event>(base, fd);

    buf_ev->register_read_cb(http_read_cb, buf_ev.get());
    buf_ev->register_write_cb(http_write_cb, buf_ev.get());
    buf_ev->register_error_cb(http_err_cb, buf_ev.get());

    buf_ev->write_string("GET /test HTTP/1.1\r\nHost: some");
    buf_ev->write_string("host\r\nConnection: close\r\n\r\n");

    buf_ev->add_read_event();
    buf_ev->add_write_event();

    base->start_dispatch();
}

int main() {
    http_basic_test();
}
