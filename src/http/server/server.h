// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "thread/thread_pool.h"
#include "base/event_base.h"
#include "http/common/request.h"
#include "http/server/server_thread.h"
#include "http/common/common.h"

namespace libevent_cpp {

using HandleCallBack = std::function<void(http_request*)>;

// HTTP 服务端封装
class http_server : public std::enable_shared_from_this<http_server> {
 public:
    http_server();
    ~http_server();

 public:
    // 设置超时时间
    inline void set_timeout(int sec) {
        timeout_ = sec;
    }

    // 设置不同 path 的回调处理函数
    inline void set_handle_cb(std::string what, HandleCallBack cb) {
        (*handle_callbacks_)[what] = cb;
    }

    // 获取线程池空闲线程数量
    inline size_t get_idle_thread_num() {
        return pool_->get_idle_thread_num();
    }

    // 重置线程池线程个数
    void resize_thread_pool(size_t thread_num);

    // http 服务端启动
    int run(const std::string& address, unsigned short port);

 private:
    // 获取当前类对象的智能指针
    std::shared_ptr<http_server> get_shared_this_ptr() {
        return http_server::shared_from_this();
    }

    // 唤醒一定数量的线程
    void wakeup_some_thread(size_t thread_count);

    // 唤醒所有线程
    void wakeup_all_thread();

    // 唤醒一个指定线程
    void wakeup_one_thread(size_t thread_num);

 private:
    // 插入线程池中的线程执行任务
    static void dispatch_task(http_server_thread* thread);

    // http 服务端监听事件处理的回调函数
    static void listen_cb(int fd, std::shared_ptr<http_server> server);

 private:
    // 服务端接收到的客户端信息
    std::shared_ptr<concurrent_queue<http_client_info>> client_info_queue_ = nullptr;
    // 服务端不同 path 的回调函数
    std::shared_ptr<std::map<std::string, HandleCallBack>> handle_callbacks_;
    // 线程池
    std::shared_ptr<thread_pool> pool_ = nullptr;
    // 事件管理类
    std::shared_ptr<event_base> base_ = nullptr;
    // 所有的线程的执行类
    std::vector<std::unique_ptr<http_server_thread>> threads_;
    // 超时事件
    int timeout_ = -1;
};

}  // namespace libevent_cpp

