#include <algorithm>
#include "event_base.h"
#include "util/log/logger.h"

libevent_cpp::event_base::event_base() {
    // 默认只有一个优先级
    init_priority(1); 
}

libevent_cpp::event_base::~event_base() {
    callback_func_map_.clear();
    active_event_queues_.clear();
    fd_map_io_event_.clear();
}

bool libevent_cpp::event_base::init_priority(int priorities) {
    if (priorities == get_active_queue_size() || priorities < 1) {
        return false;
    }
    if (!active_event_queues_.empty() && priorities != get_active_queue_size()) {
        active_event_queues_.clear();
    }
    active_event_queues_.resize(priorities);
    return true; 
}

bool libevent_cpp::event_base::add_event(const std::shared_ptr<event>& ev) {
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        // 如果此事件既不可读、又不可写，则不加入
        if (io_ev->is_event_type_removeable()) {
            logger::error("event_base::add_event warn, event with no read or write");
            return false;
        }
        fd_map_io_event_[io_ev->fd_] = io_ev;
        return add(io_ev); 
    } else {
        logger::error("event_base::add_event error, incorrect event type");
        return false;
    }
}

bool libevent_cpp::event_base::remove_event(const std::shared_ptr<event>& ev) {
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        if (fd_map_io_event_.count(io_ev->fd_) == 0) {
            return true; 
        }
        return remove(io_ev);
    } else {
        logger::error("event_base::remove_event error, incorrect event type");
        return false;
    }
}

void libevent_cpp::event_base::push_event_active_queue(std::shared_ptr<event> ev) {
    active_event_queues_[ev->priority_].push(ev);
    ev->set_active_status();
}

bool libevent_cpp::event_base::start_dispatch() {
    bool done = false;
    while (!done) {
        logger::debug("event_base::start_dispatch");
        if (!get_io_event_number()) {
            logger::warn("event_base::start_dispatch hava no events, just exit");
            return false; 
        }
        bool res = dispatch(nullptr);
        if (!res) {
            logger::error("event_base::start_dispatch dispatch failed");
            return false; 
        }
        // 如果有活跃事件
        if(get_active_event_number()) {
            process_active_events(); 
        }
    }
    return true; 
}

void libevent_cpp::event_base::process_active_events() {
    if (active_event_queues_.empty()) return; 
    auto it = std::find_if(active_event_queues_.begin(), active_event_queues_.end(), [](decltype(active_event_queues_[0]) q) { return !q.empty(); });
    auto &q = *it;
    while (!q.empty()) {
        auto ev = q.front();
        q.pop();
        // 取到 event 对应的回调函数，执行回调函数 
        auto f = callback_func_map_[ev->event_id_];
        if (f) {
            (*f)();
        }
        ev->clear_active_status();
    }
}