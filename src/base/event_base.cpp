// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <string.h>
#include <algorithm>
#include "base/event_base.h"
#include "util/util_logger.h"

// 是否捕获了信号，0 代表没有
volatile sig_atomic_t libevent_cpp::event_base::is_caught_signal_ = 0;
// 捕获信号的数组，存储每个信号被触发的个数
std::vector<int> libevent_cpp::event_base::caught_signal_vec_ = {};

libevent_cpp::event_base::event_base() {
    // 默认只有一个优先级
    init_priority(1);
    // 将信号集合置空
    sigemptyset(&signal_set_);
    // NSIG 表示当前系统支持的最多信号数
    // SIGRTMAX (_NSIG-1)则表示信号的最大值,而与之相对应的 SIGRTMIN 却不是表示信号的最小值，其表示可靠信号的最小值
    // 按照信号的可靠性，可将信号分为“可靠信号（实时信号）”和“不可靠信号（非实时信号）”，
    // 以 SIGRTMIN 为界限，值小于 SIGRTMIN 的信号为不可靠信号，其继承于早期的 UNIX 系统，
    // SIGRTMIN 到 SIGRTMAX 之间的为可靠信号
    // 不可靠信号有两点需要注意：
    // 一是其在执行完自定义信号处理函数后会将信号处理方式重置为系统默认方式（如果要多次处理同一个信号，则要多次按照信号处理函数，
    // 不过好像后期的Linux对这点做了改进而无需重新安装）。
    // 二是不可靠信号不支持排队，其有可能会出现信号丢失的情况
    caught_signal_vec_.resize(NSIG);
}

libevent_cpp::event_base::~event_base() {
    clean_up();
}

int libevent_cpp::event_base::add_event(const std::shared_ptr<event>& ev) {
    // 先标识此事件是存活的
    ev->is_alive = true;
    // 如果是 IO_EVENT 事件
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        // 如果此事件既不可读、又不可写，则不加入
        if (io_ev->is_event_type_removeable()) {
            logger::error("event_base::add_event warn, event with no read or write");
            return -1;
        }
        io_event_map_[io_ev->fd_] = io_ev;
        return add(io_ev);
    } else if (std::dynamic_pointer_cast<signal_event>(ev)) {
        // 事件为 SIGNAL_EVENT 事件
        auto sig_ev = std::dynamic_pointer_cast<signal_event>(ev);
        signal_event_list_.emplace_back(sig_ev);
        // 将此信号添加到信号集中
        return sigaddset(&signal_set_, sig_ev->get_signal());
    } else if (std::dynamic_pointer_cast<time_event>(ev)) {
        // 事件为 TIME_EVENT 事件
        auto time_ev = std::dynamic_pointer_cast<time_event>(ev);
        time_event_set_.emplace(time_ev);
        return 0;
    } else {
        logger::error("event_base::add_event error, incorrect event type: %s", typeid(ev).name());
        return -2;
    }
}

int libevent_cpp::event_base::remove_event(const std::shared_ptr<event>& ev) {
    if (ev->is_alive == false) {
        return 0;
    }
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        ev->is_alive = false;
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        if (io_event_map_.count(io_ev->fd_) == 0) {
            return 0;
        }
        int res = remove(io_ev);
        if (io_ev->is_event_type_removeable()) {
            io_event_map_.erase(io_ev->fd_);
            io_ev->is_alive = false;
        }
        return res;
    } else if (std::dynamic_pointer_cast<signal_event>(ev)) {
        ev->is_alive = false;
        auto sig_ev = std::dynamic_pointer_cast<signal_event>(ev);
        signal_event_list_.remove(sig_ev);
        sigdelset(&signal_set_, sig_ev->get_signal());
        // 将注册的信号处理动作删除
        return sigaction(sig_ev->get_signal(), (struct sigaction*)SIG_DFL, nullptr);
    } else if (std::dynamic_pointer_cast<time_event>(ev)) {
        ev->is_alive = false;
        auto time_ev = std::dynamic_pointer_cast<time_event>(ev);
        time_event_set_.erase(time_ev);
        return 0;
    } else {
        logger::error("event_base::remove_event error, incorrect event type");
        return -1;
    }
}

int libevent_cpp::event_base::start_dispatch() {
    // 计算我们需要等待的事件
    if (recalc() < 0) {
        return -1;
    }
    bool done = false;
    while (!done) {
        logger::debug("event_base::start_dispatch");
        // 如果需要退出
        if (is_terminated) {
            logger::info("exit dispatch, terminated is true");
            is_terminated = false;
            break;
        }
        // 获取到活跃事件的个数
        int active_event_num = get_active_event_number();
        // 如果没有 IO 事件、信号事件、定时事件，以及活跃事件，则直接退出
        if (io_event_map_.empty() && signal_event_list_.empty()
            && time_event_set_.empty() && active_event_num <= 0) {
            logger::warn("event_base::start_dispatch hava no events, just exit");
            return -1;
        }
        int res = 0;
        // 如果没有活跃事件，需要进行 dispatch
        if (active_event_num == 0) {
            // 没有定时事件
            if (time_event_set_.empty()) {
                res = dispatch(nullptr);
            } else {
                // 有定时事件
                struct timeval now, off;
                gettimeofday(&now, nullptr);
                auto time_ev = *time_event_set_.begin();
                auto tm = time_ev->get_timeout();
                if (timercmp(&tm, &now, >)) {
                    timersub(&tm, &now, &off);
                    res = dispatch(&off);
                }
            }
        }
        if (res < 0) {
            logger::error("event_base::start_dispatch dispatch failed");
            return -2;
        }
        // 如果有定时事件
        if (!time_event_set_.empty()) {
            process_timeout_events();
        }
        // 如果有活跃事件
        if (get_active_event_number()) {
            process_active_events();
        }
        // 在重新循环前，保证这个时间段被触发的信号不会被丢失
        if (recalc() < 0) {
            return -3;
        }
    }
    // 结束前执行清理动作
    clean_up();
    return 0;
}

void libevent_cpp::event_base::clean_up() {
    active_event_queues_.clear();
    signal_event_list_.clear();
    time_event_set_.clear();
    io_event_map_.clear();
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
    ev->is_alive = false;
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
        auto cb_func = ev->get_callback();
        if (cb_func) {
            (*cb_func)();
        }
        ev->set_inactive_status();
    }
}

void libevent_cpp::event_base::process_timeout_events() {
    struct timeval now;
    gettimeofday(&now, nullptr);
    for (auto iter = time_event_set_.begin(); iter != time_event_set_.end(); ) {
        auto ev = *iter;
        auto tm = ev->get_timeout();
        if (timercmp(&tm, &now, >)) {
            break;
        }
        iter = time_event_set_.erase(iter);
        push_event_active_queue(ev, 1);
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
    // 将捕获信号的数组全部置为0，表明所有注册信号被捕获次数都为0
    std::fill(caught_signal_vec_.begin(), caught_signal_vec_.end(), 0);
    is_caught_signal_ = 0;
}

int libevent_cpp::event_base::recalc_signal_event() {
    if (signal_event_list_.empty()) {
        return 0;
    }
    // 阻塞住信号集中的所有信号
    if (sigprocmask(SIG_BLOCK, &signal_set_, nullptr) < 0) {
        logger::error("sigprocmask failed, err: %s", strerror(errno));
        return -1;
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sa.sa_mask = signal_set_;
    // 由此信号中断的系统调用自动重启
    sa.sa_flags |= SA_RESTART;
    for (const auto& ev : signal_event_list_) {
        auto sig = ev->get_signal();
        // 判断此信号值是否合法
        if (sig < 0 || sig >= NSIG) {
            logger::error("invalid sig num: %d", sig);
            continue;
        }
        if (sigaction(sig, &sa, nullptr) < 0) {
            logger::error("sigaction failed, err: %s", strerror(errno));
            continue;
        }
    }
    return 0;
}

int libevent_cpp::event_base::deliver_signal_event() {
    if (signal_event_list_.empty()) {
        return 0;
    }
    int res = sigprocmask(SIG_UNBLOCK, &signal_set_, nullptr);
    if (res < 0) {
        logger::error("sigprocmask SIG_UNBLOCK failed, err: %s", strerror(errno));
        return -1;
    }
    return 0;
}
