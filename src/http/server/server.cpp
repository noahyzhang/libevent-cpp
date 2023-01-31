// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <algorithm>
#include "http/server/server.h"
#include "base/epoll_base.h"
#include "util/util_network.h"

libevent_cpp::http_server::http_server() {
    client_info_queue_ = std::make_shared<concurrent_queue<http_client_info>>();
    handle_callbacks_ = std::make_shared<std::map<std::string, HandleCallBack>>();
    base_ = std::make_shared<libevent_cpp::epoll_base>();
    pool_ = std::make_shared<libevent_cpp::thread_pool>();
}

libevent_cpp::http_server::~http_server() {
    for (size_t i = 0; i < threads_.size(); i++) {
        threads_[i]->set_terminate();
    }
}

void libevent_cpp::http_server::dispatch_task(http_server_thread* thread) {
    thread->dispatch();
}

void libevent_cpp::http_server::resize_thread_pool(size_t thread_num) {
    pool_->reset_thread_num(thread_num);
    int cur_threads = threads_.size();
    if (cur_threads <= thread_num) {
        for (int i = cur_threads; i < thread_num; i++) {
            threads_.emplace_back(std::unique_ptr<http_server_thread>(
                new http_server_thread(client_info_queue_, handle_callbacks_)));
        }
    } else {
        threads_.resize(thread_num);
    }

    for (int i = 0; i < thread_num; i++) {
        pool_->push(dispatch_task, threads_[i].get());
    }
}

void libevent_cpp::http_server::wakeup_some_thread(size_t thread_count) {
    unsigned int rand_num;
    for (size_t i = 0; i < thread_count; ++i) {
        unsigned int tm = time(nullptr);
        wakeup_one_thread(rand_r(&tm) % threads_.size());
    }
}

void libevent_cpp::http_server::wakeup_all_thread() {
    for (size_t i = 0; i < threads_.size(); ++i) {
        wakeup_one_thread(i);
    }
}

void libevent_cpp::http_server::wakeup_one_thread(size_t thread_num) {
    if (thread_num > threads_.size()) return;
    threads_[thread_num]->wakeup();
}

void libevent_cpp::http_server::listen_cb(int fd, std::shared_ptr<http_server> server) {
    std::string host;
    int port;
    int peer_fd = util_network::accept_socket(fd, std::make_shared<std::string>(host), std::make_shared<int>(port));

    logger::info("http server accept new client with fd: %d, host: %s, port: %d", peer_fd, host.c_str(), port);

    server->client_info_queue_->push(http_client_info{peer_fd, host, port});
    // 唤醒线程来处理
    server->wakeup_some_thread(2);
}

int libevent_cpp::http_server::run(const std::string& address, unsigned short port) {
    if (threads_.size() == 0) {
        resize_thread_pool(4);
    }
    int fd = util_network::bind_socket(address, port, true);
    if (fd < 0) {
        logger::error("bind socket failed, address: %s, port: %d", address, port);
        return -1;
    }
    if (util_network::listen_fd(fd) < 0) {
        return -2;
    }
    // 使用一个读事件去 accept 文件描述符
    auto ev = create_event<io_event>(fd, READ_ONLY);
    base_->register_callback(ev, listen_cb, fd, get_shared_this_ptr());
    ev->set_persistent();
    base_->add_event(ev);

    logger::info("http server listening on fd: %d, bound to port: %d, awaiting connections.");

    base_->start_dispatch();
    return 0;
}
