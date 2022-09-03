#include "client.h"
#include "base/epoll_base.h"
#include "util/net/network.h"

libevent_cpp::http_client::http_client() {
    base_ = std::make_shared<epoll_base>();
}

std::unique_ptr<libevent_cpp::http_client_connection> libevent_cpp::http_client::make_connection(
    const std::string& server_address, unsigned int server_port) {
    int fd = util_network::get_nonblock_socket();
    auto conn = std::unique_ptr<http_client_connection>(new http_client_connection(base_, fd, timeout_))); 
    conn->set_server_addr(server_address);
    conn->set_server_port(server_port);
    if (conn->connect() < 0) {
        logger::error("make_connection failed, client connect to fd: %d err", fd);
        exit(-1); 
    }
    return conn; 
}

void libevent_cpp::http_client::run() {
    base_->start_dispatch();
}