// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "thread/thread_pool.h"
#include "base/event_base.h"
#include "common/request.h"
#include "http/server/server_thread.h"
#include "thread/concurrent_map.h"

namespace libevent_cpp {

// 客户端信息
struct http_client_info {
    int peer_fd_;
    std::string host_;
    int port_;
};

using HandleCallBack = std::function<void(http_request*)>;

class http_server {
 public:
    http_server();
    ~http_server();

    void resize_thread_pool(size_t thread_num);
    int run(const std::string& address, unsigned short port);

    inline size_t get_idle_thread_num() {
        return pool_->get_idle_thread_num();
    }
    inline void set_timeout(int sec) {
        timeout_ = sec;
    }
    inline void set_handle_cb(std::string what, HandleCallBack cb) {
        handle_callbacks_[what] = cb;
    }

 public:
    concurrent_queue<std::unique_ptr<http_client_info>> client_info_queue_;

 private:
    static void dispatch_task(http_server_thread* thread);
    static void listen_cb(int fd, http_server* server);

 private:
    std::shared_ptr<thread_pool> pool_ = nullptr;
    std::shared_ptr<event_base> base_ = nullptr;
    std::vector<std::unique_ptr<http_server_thread>> threads_;
    std::map<std::string, HandleCallBack> handle_callbacks_;
    int timeout_ = -1;
};

}  // namespace libevent_cpp

