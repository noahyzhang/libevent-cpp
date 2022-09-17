#ifndef LIBEVENT_CPP_THREAD_POOL_H
#define LIBEVENT_CPP_THREAD_POOL_H

#include <vector>
#include <memory>
#include <atomic>
#include <mutex> 
#include <condition_variable>
#include <thread> 
#include <functional>
#include <queue>
#include <future>
#include "concurrent_map.h"

namespace libevent_cpp {

class thread_pool {
private:  
    std::vector<std::unique_ptr<std::thread>> threads_;
    std::vector<std::shared_ptr<std::atomic<bool>>> flags_;
    concurrent_queue<std::function<void(int id)>*> queue_; 
    std::atomic<bool> is_done_;
    std::atomic<bool> is_stop_;
    std::atomic<size_t> waiting_threads_; // 处于等待中的线程数量 
    std::mutex mutex_;
    std::condition_variable cv_; 

public:  
    thread_pool() { init(); }
    thread_pool(int thread_num) { init(); reset_thread_num(thread_num); }

    template <typename F, typename... Rest>
    auto push(F&& f, Rest&&... rest) ->std::function<decltype(f(0, rest...))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(
            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
        );
        auto func = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
        });
        queue_.push(func);
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.notify_one();
        return pck->get_future();
    }

    template <typename F> 
    auto push(F&& f) ->std::future<decltype(f(0))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0))>>(std::forward<F>(f)); 
        auto func = std::function<void(int id)>([pck](int id) {
            (*pck)(id);
        });
        queue_.push(func);
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.notify_one();
        return pck->get_future(); 
    }

    inline size_t get_thread_num() const { return threads_.size(); }
    inline size_t get_idle_thread_num() const {
        return waiting_threads_;
    }

    void reset_thread_num(int thread_num);
    void clear_queue();
    std::function<void(int)> pop();
    // 等待所有线程完成并停止线程 
    // 如果 is_wait 为 true，则队列中所有函数将被运行；如果 is_wait 为 false，队列将被清除，而且不运行函数 
    void stop(bool is_wait = false);

private:  
    void init() {
        waiting_threads_ = 0;
        is_stop_ = false;
        is_done_ = false; 
    }
    void set_thread(int i); 
}; 

} // libevent_cpp 

#endif // LIBEVENT_CPP_THREAD_POOL_H