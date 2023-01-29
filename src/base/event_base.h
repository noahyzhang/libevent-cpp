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
#include <set>
#include "event/event.h"
#include "event/io_event.h"
#include "event/signal_event.h"
#include "event/time_event.h"

namespace libevent_cpp {

using Callback = std::function<void()>;

// 事件管理基类
class event_base {
 public:
    // 定义公共的事件管理类方法
    virtual int init() { return 0; }
    virtual int add(std::shared_ptr<io_event>) { return 0; }
    virtual int remove(std::shared_ptr<io_event>) { return 0; }
    virtual int dispatch(struct timeval*) { return 0; }

 public:
    event_base();
    virtual ~event_base();

 public:
    // 添加事件
    int add_event(const std::shared_ptr<event>& ev);
    // 移除事件
    int remove_event(const std::shared_ptr<event>& ev);
    // 开始调度
    int start_dispatch();

    // 将事件添加到活跃队列中
    void push_event_active_queue(std::shared_ptr<event> ev, size_t call_num);
    // 处理活跃事件
    void process_active_events();
    // 初始化活跃队列的优先级
    int init_priority(int priorities);
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

 private:
    // 定义定时事件在容器中存放的顺序
    struct cmp_time_event {
        bool operator()(const std::shared_ptr<time_event> tm_ev1, const std::shared_ptr<time_event> tm_ev2) const {
            return timercmp(&tm_ev1->get_timeout(), &tm_ev2->get_timeout(), <);
        }
    };

 public:
    // 信号事件的集合
    sigset_t signal_event_mask_;
    static volatile sig_atomic_t caught_num_;  // 捕获次数

 protected:
    // 处理信号事件
    void process_signal_event();
    // 
    int recalc_signal_event();

 private:
    bool is_terminated = false;
    // 存放活跃事件的队列
    std::vector<std::queue<std::shared_ptr<event>>> active_event_queues_;
    // 存放回调函数的 map
    std::map<int, std::shared_ptr<Callback>> callback_func_map_;
    // 存放信号事件的链表
    std::list<std::shared_ptr<signal_event>> signal_event_list_;
    // 存放定时事件的容器，按照时间大小排序
    std::set<std::shared_ptr<time_event>, cmp_time_event> time_event_st_;
    // 存放捕获信号的数组
    static std::vector<int> caught_signal_vec_;

 protected:
    // 存放 fd 与事件的 map
    std::map<int, std::shared_ptr<io_event>> fd_map_io_event_;
};

}  // namespace libevent_cpp
