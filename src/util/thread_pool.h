// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <queue>
#include <future>
#include <utility>
#include "util/concurrent_queue.h"

namespace libevent_cpp {

// 线程池
class thread_pool {
 public:
    thread_pool() = default;
    explicit thread_pool(int thread_num) {
        reset_thread_num(thread_num);
    }
    ~thread_pool() {
        threads_.clear();
        flags_.clear();
        queue_.clear();
    }

    // 插入一个任务
    template <typename F, typename... Rest>
    std::future<typename std::result_of<F(Rest...)>::type>  push(F&& f, Rest&&... rest) {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...));
        auto func = new std::function<void(int)>([pck](int id) {
            (*pck)(id);
        });
        queue_.push(func);
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.notify_one();
        return pck->get_future();
    }

    // 插入一个任务
    template <typename F>
    auto push(F&& f) ->std::future<decltype(f(0))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0))>>(std::forward<F>(f));
        auto func = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
        });
        queue_.push(func);
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.notify_one();
        return pck->get_future();
    }

    // 获取线程的个数
    inline size_t get_thread_num() const { return threads_.size(); }

    // 获取空闲线程的个数
    inline size_t get_idle_thread_num() const {
        return waiting_threads_;
    }

    // 重置线程池中线程的个数
    void reset_thread_num(size_t thread_num);

    // 弹出一个任务
    const std::shared_ptr<std::function<void(int)>> pop();

    // 等待所有线程完成并停止线程
    // 如果 is_wait 为 true，则队列中所有任务都将被运行；如果 is_wait 为 false，队列将被清除，而且不运行函数
    void stop(bool is_wait = false);

 private:
    // 创建线程
    void create_thread(size_t thread_num);
    // 清空队列中所有任务
    void clear_queue();

 private:
    // 所有的线程
    std::vector<std::unique_ptr<std::thread>> threads_;
    // 每个线程的标志，即是否允许处理任务
    std::vector<std::shared_ptr<std::atomic<bool>>> flags_;
    // 线程安全的队列，存储任务
    concurrent_queue<std::function<void(int)>> queue_;
    // 退出标志，允许退出前线程仍旧处理任务
    std::atomic<bool> is_done_{false};
    // 退出标志，线程退出前可不处理队列中的任务
    std::atomic<bool> is_stop_{false};
    // 处于等待中的线程数量
    std::atomic<size_t> waiting_threads_{0};
    std::mutex mutex_;
    std::condition_variable cv_;
};

}  // namespace libevent_cpp
