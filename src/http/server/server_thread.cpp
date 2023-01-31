// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <signal.h>
#include <utility>
#include <string>
#include "base/epoll_base.h"
#include "util/util_linux.h"
#include "http/common/common.h"
#include "http/server/server_thread.h"

namespace libevent_cpp {

http_server_thread::http_server_thread(
    std::shared_ptr<concurrent_queue<http_client_info>> client_info_queue,
    std::shared_ptr<std::map<std::string, HandleCallBack>> handle_callbacks)
    : client_info_queue_(client_info_queue),
      handle_callbacks_(handle_callbacks) {
    base_ = std::make_shared<epoll_base>();
    // 使用内核提供的通知事件进行注册
    io_ev_ = create_event<io_event>(util_linux::create_eventfd(), READ_ONLY);
    // TODO
    base_->register_callback(io_ev_, get_connections, io_ev_, this);
    base_->add_event(io_ev_);

    // 处理 SIGPIPE 信号（写已经不再连接的套接字/管道）
    pipe_sig_ev_ = create_event<signal_event>(SIGPIPE);
    // TODO
    base_->register_callback(pipe_sig_ev_, ev_sigpipe_handler, pipe_sig_ev_);
    base_->add_event(pipe_sig_ev_);
}

http_server_thread::~http_server_thread() {
    base_->clean_io_event(io_ev_);
}

void http_server_thread::dispatch() {
    base_->start_dispatch();
}

void http_server_thread::set_terminate() {
    base_->set_terminated();
}

void http_server_thread::wakeup() {
    static std::string msg = "0x123456";
    size_t n = write(io_ev_->fd_, msg.c_str(), msg.length());
    if (n <= 0) {
        logger::error("wakeup failed, write eror");
    }
}

std::shared_ptr<http_server_thread> http_server_thread::get_shared_this_ptr() {
    return http_server_thread::shared_from_this();
}

std::unique_ptr<http_server_connection> http_server_thread::get_empty_connection() {
    if (empty_queue_.empty()) {
        return std::unique_ptr<http_server_connection>(new http_server_connection(base_, -1, handle_callbacks_));
    }
    auto conn = std::move(empty_queue_.front());
    empty_queue_.pop();
    return conn;
}

void http_server_thread::get_connections(std::shared_ptr<io_event> ev, std::shared_ptr<http_server_thread> thread) {
    // 先读取出 IO 通知事件中的信息
    read_wake_msg(ev->fd_);
    // 如果有连接已经被关闭了，则将此连接加入到 empty_queue 中
    for (auto iter = thread->connection_list_.begin(); iter != thread->connection_list_.end(); iter++) {
        if (iter->get()->is_closed()) {
            auto conn = std::move(*iter);
            iter = thread->connection_list_.erase(iter);
            iter--;
            conn->reset();
            thread->empty_queue_.push(std::move(conn));
        }
    }
    // 从客户端连接队列中取出客户端连接
    std::shared_ptr<http_client_info> client_info;
    while (thread->client_info_queue_->pop(client_info)) {
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

int http_server_thread::read_wake_msg(int fd) {
    char buf[32];
    if (read(fd, buf, sizeof(buf)) <= 0) {
        logger::warn("read fd: %d failed", fd);
        return -1;
    }
    return 0;
}

}  // namespace libevent_cpp
