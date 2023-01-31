// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <queue>
#include <mutex>
#include <memory>
#include <utility>

namespace libevent_cpp {

// 线程安全的队列
template <typename T>
class concurrent_queue {
 public:
    void push(T const& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(value);
    }
    void push(T const&& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push(std::forward<T>(value));
    }
    bool pop(std::shared_ptr<T> value) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = queue_.front();
        queue_.pop();
        return true;
    }
    bool empty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    bool clear() {
        while (!queue_.empty()) {
            queue_.pop();
        }
    }

 private:
    std::queue<T> queue_;
    std::mutex mutex_;
};

}  // namespace libevent_cpp
