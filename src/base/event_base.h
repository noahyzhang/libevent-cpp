// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <signal.h>
#include <time.h>
#include <vector>
#include <queue>
#include <map>
#include <memory>
#include <functional>
#include <utility>
#include <list>
#include "event/event.h"
#include "event/io_event.h"
#include "event/signal_event.h"

namespace libevent_cpp {

using Callback = std::function<void()>;

class event_base {
 public:
    virtual bool init() { return true; }
    virtual bool add(std::shared_ptr<io_event>) { return true; }
    virtual bool remove(std::shared_ptr<io_event>) { return true; }
    virtual bool dispatch(struct timeval*) { return true; }

 public:
    event_base();
    virtual ~event_base();

    // 添加事件
    bool add_event(const std::shared_ptr<event>& ev);
    // 移除事件
    bool remove_event(const std::shared_ptr<event>& ev);
    // 将事件添加到活跃队列中
    void push_event_active_queue(std::shared_ptr<event> ev, size_t call_num);
    // 开始调度
    bool start_dispatch();
    // 处理活跃事件
    void process_active_events();
    // 初始化活跃队列的优先级
    bool init_priority(int priorities);
    // 清理 io 事件
    void clean_io_event(const std::shared_ptr<io_event>& ev);

    // 注册回调函数
    template <typename E, typename F, typename... Rest>
    void register_callback(E &&ev, F &&f, Rest &&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        callback_func_map_[ev->event_id_] = std::make_shared<Callback>([task]() { task(); });
    }
    // 获取活跃事件队列的队列个数，处于相同队列的事件是同一优先级
    inline int get_active_queue_size() const {
        return active_event_queues_.size();
    }
    // 获取活跃事件的个数
    inline int get_active_event_number() {
        int res = 0;
        for (const auto& aq : active_event_queues_) {
            res += aq.size();
        }
        return res;
    }
    // 获取 io 事件的个数
    inline int get_io_event_number() {
        return fd_map_io_event_.size();
    }
    // 设置结束标识符
    inline void set_terminated() {
        is_terminated = true;
    }

 public:
    static volatile sig_atomic_t caught_num_;  // 捕获次数

 protected:
    // 处理信号事件
    void process_signal_event();
    // 
    int recalc_signal_event();

 private:
    bool is_terminated = false;
    std::vector<std::queue<std::shared_ptr<event>>> active_event_queues_;  // 存放活跃事件的队列
    std::map<int, std::shared_ptr<Callback>> callback_func_map_;  // 存放回调函数的 map
    std::list<std::shared_ptr<signal_event>> signal_event_list_;  // 存放信号事件的链表
    static std::vector<int> caught_signal_vec_;  // 存放捕获信号的数组

 protected:
    std::map<int, std::shared_ptr<io_event>> fd_map_io_event_;  // 存放 fd 与事件的 map
};

}  // namespace libevent_cpp
