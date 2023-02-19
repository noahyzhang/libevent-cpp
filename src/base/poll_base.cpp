// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <memory>
#include "base/poll_base.h"
#include "util/util_logger.h"

int libevent_cpp::poll_base::add(std::shared_ptr<io_event> ev) {
    struct pollfd* pfd = fd_map_poll_[ev->fd_];
    if (!pfd) {
        pfd = new struct pollfd();
        pfd->fd = ev->fd_;
        pfd->events = 0;
        pfd->revents = 0;
        fd_map_poll_[ev->fd_] = pfd;
    }
    if (ev->is_event_type_readable()) {
        pfd->events |= POLLIN;
    }
    if (ev->is_event_type_writeable()) {
        pfd->events |= POLLOUT;
    }
    return 0;
}

int libevent_cpp::poll_base::remove(std::shared_ptr<io_event> ev) {
    if (ev->is_event_type_removeable()) {
        delete fd_map_poll_[ev->fd_];
        fd_map_poll_.erase(ev->fd_);
    } else {
        struct pollfd* pfd = fd_map_poll_[ev->fd_];
        if (pfd == nullptr) {
            return -1;
        }
        if (!ev->is_read_event_active_status()) {
            pfd->events &= ~POLLIN;
        }
        if (!ev->is_write_event_active_status()) {
            pfd->events &= ~POLLOUT;
        }
    }
    return 0;
}

int libevent_cpp::poll_base::recalc() {
    recalc_signal_event();
}

int libevent_cpp::poll_base::dispatch(struct timeval* tv) {
    // 释放那些被阻塞的信号事件
    if (deliver_signal_event() < 0) {
        return -1;
    }
    int timeout = -1;
    if (tv) {
        // timeout 单位为毫秒。小于 1 毫秒的数值当作 1 毫秒
        timeout = tv->tv_sec * 1000 + (tv->tv_usec + 999) / 1000;
    }
    int nfds = fd_map_poll_.size();
    struct pollfd fds[nfds];
    int i = 0;
    for (const auto kv : fd_map_poll_) {
        fds[i++] = *kv.second;
    }
    int res = poll(fds, nfds, timeout);
    // 在处理已就绪的事件前，先阻塞住已注册的信号事件
    if (recalc_signal_event() < 0) {
        return -1;
    }
    if (res < 0) {
        if (errno != EINTR) {
            logger::error("poll_base::dispatch poll err");
            return -1;
        }
        // poll 被中断，此次 dispatch 结束前处理已经注册的信号
        process_signal_event();
        return 0;
    } else if (is_caught_signal_) {
        // 在 poll 这段时间中捕获的信号，执行信号处理事件
        process_signal_event();
    }
    if (res == 0) return 0;  // 如果没有就绪的文件描述符，直接返回
    int what = 0;
    for (int i = 0; i < nfds; i++) {
        what = fds[i].revents;
        auto io_ev = io_event_map_[fds[i].fd];
        if (what && io_ev) {
            io_ev->clear_event_active_status();
            if (what & (POLLHUP | POLLERR)) {
                what |= POLLIN | POLLOUT;
            }
            if ((what & POLLIN) && io_ev->is_event_type_readable()) {
                io_ev->enable_read_event_status_active();
            }
            if ((what & POLLOUT) && io_ev->is_event_type_writeable()) {
                io_ev->enable_write_event_status_active();
            }
            if (io_ev->is_read_event_active_status() || io_ev->is_write_event_active_status()) {
                // 如果是非持久性的事件，则从事件队列中删除
                if (!io_ev->is_persistent()) {
                    remove_event(io_ev);
                }
                push_event_active_queue(io_ev, 1);
            }
        }
    }
    return 0;
}
