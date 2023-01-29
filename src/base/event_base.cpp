// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <algorithm>
#include "base/event_base.h"
#include "util/util_logger.h"

volatile sig_atomic_t libevent_cpp::event_base::caught_num_ = 0;
std::vector<int> libevent_cpp::event_base::caught_signal_vec_ = {};

libevent_cpp::event_base::event_base() {
    // 默认只有一个优先级
    init_priority(1);
    // 将集合置空
    sigemptyset(&signal_event_mask_);
}

libevent_cpp::event_base::~event_base() {
    callback_func_map_.clear();
    active_event_queues_.clear();
    fd_map_io_event_.clear();
}

int libevent_cpp::event_base::add_event(const std::shared_ptr<event>& ev) {
    // 如果是 IO_EVENT 事件
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        // 如果此事件既不可读、又不可写，则不加入
        if (io_ev->is_event_type_removeable()) {
            logger::error("event_base::add_event warn, event with no read or write");
            return -1;
        }
        fd_map_io_event_[io_ev->fd_] = io_ev;
        return add(io_ev);
    } else if (std::dynamic_pointer_cast<signal_event>(ev)) {
        // 事件为 SIGNAL_EVENT 事件
        auto sig_ev = std::dynamic_pointer_cast<signal_event>(ev);
        signal_event_list_.emplace_back(sig_ev);
        // 将此信号添加到信号集中
        return sigaddset(&signal_event_mask_, sig_ev->get_signal());
    } else if (std::dynamic_pointer_cast<time_event>(ev)) {
        // 事件为 TIME_EVENT 事件
        auto time_ev = std::dynamic_pointer_cast<time_event>(ev);
        time_event_st_.emplace(time_ev);
        return 0;
    } else {
        logger::error("event_base::add_event error, incorrect event type: %s", typeid(ev).name());
        return -2;
    }
}

int libevent_cpp::event_base::remove_event(const std::shared_ptr<event>& ev) {
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        if (fd_map_io_event_.count(io_ev->fd_) == 0) {
            return 0;
        }
        return remove(io_ev);
    } else if (std::dynamic_pointer_cast<signal_event>(ev)) {
        auto sig_ev = std::dynamic_pointer_cast<signal_event>(ev);
        signal_event_list_.remove(sig_ev);
        sigdelset(&signal_event_mask_, sig_ev->get_signal());
        // 将注册的信号处理动作删除
        return sigaction(sig_ev->get_signal(), (struct sigaction*)SIG_DFL, nullptr);
    } else if (std::dynamic_pointer_cast<time_event>(ev)) {
        auto time_ev = std::dynamic_pointer_cast<time_event>(ev);
        time_event_st_.erase(time_ev);
        return 0;
    } else {
        logger::error("event_base::remove_event error, incorrect event type");
        return -1;
    }
}

int libevent_cpp::event_base::start_dispatch() {
    bool done = false;
    while (!done) {
        logger::debug("event_base::start_dispatch");
        if (!get_io_event_number()) {
            logger::warn("event_base::start_dispatch hava no events, just exit");
            return -1;
        }
        if (dispatch(nullptr) < 0) {
            logger::error("event_base::start_dispatch dispatch failed");
            return -2;
        }
        // 如果有活跃事件
        if (get_active_event_number()) {
            process_active_events();
        }
    }
    return 0;
}

int libevent_cpp::event_base::init_priority(int priorities) {
    if (priorities == get_active_queue_size() || priorities < 1) {
        return -1;
    }
    if (!active_event_queues_.empty() && priorities != get_active_queue_size()) {
        active_event_queues_.clear();
    }
    active_event_queues_.resize(priorities);
    return 0;
}

void libevent_cpp::event_base::clean_io_event(const std::shared_ptr<io_event>& ev) {
    ev->disable_read_event_status_active();
    ev->disable_write_event_status_active();
    remove_event(ev);
    // TODO 
}

void libevent_cpp::event_base::push_event_active_queue(std::shared_ptr<event> ev, size_t call_num) {
    ev->call_num_ = call_num;
    active_event_queues_[ev->priority_].push(ev);
    ev->set_active_status();
}

void libevent_cpp::event_base::process_active_events() {
    if (active_event_queues_.empty()) return;
    auto it = std::find_if(active_event_queues_.begin(), active_event_queues_.end(),
            [](decltype(active_event_queues_[0]) q) { return !q.empty(); });
    auto &q = *it;
    while (!q.empty()) {
        auto ev = q.front();
        q.pop();
        // 取到 event 对应的回调函数，执行回调函数
        auto f = callback_func_map_[ev->event_id_];
        if (f) {
            (*f)();
        }
        ev->set_inactive_status();
    }
}

void libevent_cpp::event_base::process_signal_event() {
    int call_num;
    auto iter = signal_event_list_.begin();
    while (iter != signal_event_list_.end()) {
        auto ev = *iter;
        call_num = caught_signal_vec_[ev->get_signal()];
        if (call_num) {
            // 如果是非持久信号事件，则需要移除这个信号事件
            if (!(ev->is_persistent())) {
                iter = signal_event_list_.erase(iter);
            }
            push_event_active_queue(ev, call_num);
        }
        iter++;
    }
    // 将捕获信号的数组清空
    caught_signal_vec_.clear();
    // std::fill(caught_signal_vec_.begin(), caught_signal_vec_.end(), nullptr);
    caught_num_ = 0;
}
