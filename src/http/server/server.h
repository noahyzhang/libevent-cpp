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

class http_server : public std::enable_shared_from_this<http_server> {
 public:
    http_server();
    ~http_server();

 public:
    // 获取当前类对象的智能指针
    std::shared_ptr<http_server> get_shared_this_ptr() {
        return http_server::shared_from_this();
    }

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

    void wakeup_some_thread(size_t thread_count);
    void wakeup_all_thread();
    void wakeup_one_thread(size_t thread_num);

 public:
    concurrent_queue<std::unique_ptr<http_client_info>> client_info_queue_;

 private:
    static void dispatch_task(http_server_thread* thread);
    static void listen_cb(int fd, std::shared_ptr<http_server> server);

 private:
    // 线程池
    std::shared_ptr<thread_pool> pool_ = nullptr;
    // 事件管理类
    std::shared_ptr<event_base> base_ = nullptr;
    std::vector<std::unique_ptr<http_server_thread>> threads_;
    std::map<std::string, HandleCallBack> handle_callbacks_;
    int timeout_ = -1;
};

}  // namespace libevent_cpp

