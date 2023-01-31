// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include "thread/thread_pool.h"

void libevent_cpp::thread_pool::create_thread(size_t thread_num) {
    std::shared_ptr<std::atomic<bool>> flag(flags_[thread_num]);
    auto f = [this, thread_num, flag]() {
        std::atomic<bool>& internal_flag = *flag;
        auto func = std::make_shared<std::function<void(int)>>();
        bool is_pop = queue_.pop(func);
        while (true) {
            while (is_pop) {
                (*func)(thread_num);
                if (internal_flag) {
                    return;
                } else {
                    is_pop = queue_.pop(func);
                }
            }
            // 如果 queue_ 为空，则阻塞等待，当 push 进一个 func 时，会唤醒条件变量
            std::unique_lock<std::mutex> lock(mutex_);
            ++waiting_threads_;
            cv_.wait(lock, [this, &func, &is_pop, &internal_flag]() {
                is_pop = queue_.pop(func);
                return is_pop || is_done_ || internal_flag;
            });
            --waiting_threads_;
            // 此时线程需要退出，即 is_done_ 或者 internal_flag 为 true
            if (!is_pop) {
                return;
            }
        }
    };
    threads_[thread_num].reset(new std::thread(f));
}

void libevent_cpp::thread_pool::reset_thread_num(size_t thread_num) {
    if (!is_stop_ && !is_done_) {
        size_t old_thread_num = threads_.size();
        // 如果新的线程数等于原来的线程数，不需要扩缩容
        if (old_thread_num == thread_num) {
            return;
        } else if (old_thread_num < thread_num) {
            // 扩容的情况
            threads_.resize(thread_num);
            flags_.resize(thread_num);
            // 设置扩容的那几个线程
            for (size_t i = old_thread_num; i < thread_num; i++) {
                // 新设置的线程的 flag 设置为 false，即可以处理任务
                flags_[i] = std::make_shared<std::atomic<bool>>(false);
                create_thread(i);
            }
        } else {
            // 缩容的情况，把将要被缩掉的线程设置 flag，使其退出
            for (size_t i = old_thread_num -1; i >= thread_num; --i) {
                *flags_[i] = true;
                threads_[i]->detach();
            }
            {
                // 通知所有线程，退出那些要被缩容掉的线程
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.notify_all();
            }
            threads_.resize(thread_num);
            flags_.resize(thread_num);
        }
    }
}

void libevent_cpp::thread_pool::clear_queue() {
    auto func = std::make_shared<std::function<void(int)>>();
    while (queue_.pop(func)) {}
}

const std::shared_ptr<std::function<void(int)>> libevent_cpp::thread_pool::pop() {
    auto func = std::make_shared<std::function<void(int)>>();
    queue_.pop(func);
    return func;
}

void libevent_cpp::thread_pool::stop(bool is_wait = false) {
    // 1. 如果设置为不等待，则直接清空队列中的所有任务
    // 2. 如果设置为等待，则等待线程执行完队列中的所有任务之后再退出
    if (!is_wait) {
        if (is_stop_) {
            return;
        }
        is_stop_ = true;
        for (int i = 0, n = get_thread_num(); i < n; ++i) {
            *flags_[i] = true;
        }
        clear_queue();
    } else {
        if (is_done_ || is_stop_) {
            return;
        }
        is_done_ = true;
    }
    // 这里通知所有线程退出，并且对线程进行回收
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.notify_all();
    }
    for (int i = 0; i < get_thread_num(); ++i) {
        if (threads_[i]->joinable()) {
            threads_[i]->join();
        }
    }
    // 存在队列中的元素未处理玩的情况，这里再次清空，以防内存泄漏
    clear_queue();
    threads_.clear();
    flags_.clear();
}
