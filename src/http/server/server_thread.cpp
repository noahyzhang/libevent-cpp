// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <signal.h>
#include <utility>
#include <string>
#include "http/server/server_thread.h"
#include "base/epoll_base.h"
#include "util/util_linux.h"

libevent_cpp::http_server_thread::http_server_thread(http_server* server)
    : server_(server) {
    base_ = std::make_shared<epoll_base>();
    io_ev_ = create_event<io_event>(util_linux::create_eventfd(), READ_ONLY);
    // TODO
    base_->register_callback(io_ev_, get_connections, io_ev_, this);
    base_->add_event(io_ev_);

    ev_sigpipe_ = create_event<signal_event>(SIGPIPE);
    // TODO
    base_->register_callback(ev_sigpipe_, ev_sigpipe_handler, ev_sigpipe_);
    base_->add_event(ev_sigpipe_);
}

libevent_cpp::http_server_thread::~http_server_thread() {
    base_->clean_io_event(io_ev_);
}

void libevent_cpp::http_server_thread::dispatch() {
    base_->start_dispatch();
}

void libevent_cpp::http_server_thread::set_terminate() {
    base_->set_terminated();
}

void libevent_cpp::http_server_thread::wakeup() {
    static std::string msg = "0x123456";
    size_t n = write(io_ev_->fd_, msg.c_str(), msg.length());
    if (n <= 0) {
        logger::error("wakeup failed, write eror");
    }
}

std::unique_ptr<libevent_cpp::http_server_connection> libevent_cpp::http_server_thread::get_empty_connection() {
    if (empty_queue_.empty()) {
        return std::unique_ptr<libevent_cpp::http_server_connection>(new http_server_connection(base_, -1, server_));
    }
    auto conn = std::move(empty_queue_.front());
    empty_queue_.pop();
    return conn;
}

void libevent_cpp::http_server_thread::get_connections(std::shared_ptr<io_event> ev, http_server_thread* thread) {
    // TODO
    for (auto iter = thread->connection_list_.begin(); iter != thread->connection_list_.end(); iter++) {
        if (iter->get()->is_closed()) {
            auto conn = std::move(*iter);
            iter = thread->connection_list_.erase(iter);
            iter--;
            conn->reset();
            thread->empty_queue_.push(std::move(conn));
        }
    }
    std::unique_ptr<http_client_info> client_info;
    while (thread->server_->client_info_queue_.pop(client_info)) {
        auto conn = thread->get_empty_connection();
        conn->set_fd(client_info->peer_fd_);
        conn->set_client_address(client_info->host_);
        conn->set_client_port(client_info->port_);
        if (conn->create_request() == 0) {
            thread->connection_list_.emplace_back(std::move(conn));
        } else {
            thread->empty_queue_.push(std::move(conn));
        }
    }
}
