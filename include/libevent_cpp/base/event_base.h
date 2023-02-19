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

// using Callback = std::function<void()>;

// 事件管理基类
class event_base {
public:
    event_base();
    virtual ~event_base();

protected:
    // 定义公共的事件管理类方法
    virtual int init() { return 0; }
    virtual int add(std::shared_ptr<io_event>) { return 0; }
    virtual int remove(std::shared_ptr<io_event>) { return 0; }
    virtual int recalc() = 0;
    virtual int dispatch(struct timeval*) { return 0; }

public:
    // 添加事件
    int add_event(const std::shared_ptr<event>& ev);
    // 移除事件
    int remove_event(const std::shared_ptr<event>& ev);
    // 开始调度
    int start_dispatch();
    // 设置结束标识符
    inline void set_terminated() {
        is_terminated = true;
    }

private:
    // 初始化活跃队列的优先级
    int init_priority(int priorities);
    // 清理 io 事件
    void clean_io_event(const std::shared_ptr<io_event>& ev);

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
    // 清理
    void clean_up();

protected:
    // 将事件添加到活跃队列中
    void push_event_active_queue(std::shared_ptr<event> ev, size_t call_num);

protected:
    // 处理活跃事件
    void process_active_events();
    // 处理超时事件
    void process_timeout_events();
    // 处理信号事件
    void process_signal_event();

    // 重新计算信号事件
    // 在每次 dispatch 前的这个过程中，如果收到已注册的信号，此时需要保存此信号，阻塞住已经发生的信号
    int recalc_signal_event();
    // 在每次 dispatch 前，先释放之前阻塞的信号
    int deliver_signal_event();

private:
    // 定义定时事件在容器中存放的顺序
    struct cmp_time_event {
        bool operator()(const std::shared_ptr<time_event> tm_ev1, const std::shared_ptr<time_event> tm_ev2) const {
            auto tm1 = tm_ev1->get_timeout();
            auto tm2 = tm_ev2->get_timeout();
            return timercmp(&tm1, &tm2, <);
        }
    };
    // 信号处理回调
    static void handle_signal(int sig) {
        caught_signal_vec_[sig]++;
        is_caught_signal_ = 1;
    }

protected:
    // 存放 fd 与 IO 事件的 map
    std::map<int, std::shared_ptr<io_event>> io_event_map_;
    // 是否捕获了信号
    static volatile sig_atomic_t is_caught_signal_;

private:
    // 存放信号事件的链表
    std::list<std::shared_ptr<signal_event>> signal_event_list_;
    // 存放定时事件的容器，按照时间先后顺序排序
    std::set<std::shared_ptr<time_event>, cmp_time_event> time_event_set_;

    // 信号集
    sigset_t signal_set_;
    // 存放捕获信号的数组
    static std::vector<int> caught_signal_vec_;

    bool is_terminated = false;
    // 存放活跃事件的队列
    std::vector<std::queue<std::shared_ptr<event>>> active_event_queues_;
};

}  // namespace libevent_cpp
