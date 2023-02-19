// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <stdlib.h>
#include <string.h>
#include <memory>
#include "base/select_base.h"
#include "util/util_logger.h"

#define MAX_SELECT_FD_SIZE 1024

libevent_cpp::select_base::~select_base() {
    delete event_read_fds_in_;
    event_read_fds_in_ = nullptr;
    delete event_write_fds_in_;
    event_write_fds_in_ = nullptr;
    delete event_read_fds_out_;
    event_read_fds_out_ = nullptr;
    delete event_write_fds_out_;
    event_write_fds_out_ = nullptr;
}

int libevent_cpp::select_base::init() {
    event_read_fds_in_ = new fd_set();
    event_write_fds_in_ = new fd_set();
    event_read_fds_out_ = new fd_set();
    event_write_fds_out_ = new fd_set();
    return 0;
}

int libevent_cpp::select_base::add(std::shared_ptr<io_event> ev) {
    // select 系统调用不能超过 1024 个文件描述符
    if (ev->fd_ > MAX_SELECT_FD_SIZE) {
        logger::error("select_base add select fd > MAX_SELECT_FD_SIZE: %d", MAX_SELECT_FD_SIZE);
        return -1;
    }
    // 更新最大的文件描述符
    if (max_fd_ < ev->fd_) {
        max_fd_ = ev->fd_;
    }
    // 如果事件可读，则设置读 fdset 的位 fd
    if (ev->is_event_type_readable()) {
        FD_SET(ev->fd_, event_read_fds_in_);
    }
    // 如果事件可写，则设置写 fdset 的位 fd
    if (ev->is_event_type_writeable()) {
        FD_SET(ev->fd_, event_write_fds_in_);
    }
    return 0;
}

int libevent_cpp::select_base::remove(std::shared_ptr<io_event> ev) {
    // 如果事件可读，则清除读 fdset 的位 fd
    if (ev->is_event_type_readable()) {
        FD_CLR(ev->fd_, event_read_fds_in_);
    }
    // 如果事件可写，则清除写 fdset 的位 fd
    if (ev->is_event_type_writeable()) {
        FD_CLR(ev->fd_, event_write_fds_in_);
    }
    return 0;
}

int libevent_cpp::select_base::recalc() {
    recalc_signal_event();
}

int libevent_cpp::select_base::dispatch(struct timeval* tv) {
    // 先释放被阻塞的信号
    if (deliver_signal_event() < 0) {
        return -1;
    }
    // 将用户设置好的 fds 拷贝，用于 select 调用
    memcpy(event_read_fds_out_, event_read_fds_in_, sizeof(fd_set));
    memcpy(event_write_fds_out_, event_write_fds_in_, sizeof(fd_set));

    int res = select(max_fd_ + 1, event_read_fds_out_, event_write_fds_out_, nullptr, tv);
    // 在处理事件前，先阻塞到所有已经注册的信号
    if (recalc_signal_event() < 0) {
        return -1;
    }
    if (res < 0) {
        if (errno != EINTR) {
            logger::error("select dispatch failed, errno: %d", errno);
            return -1;
        }
        // 当被 EINTR 中断，先执行已经注册的信号事件
        process_signal_event();
        logger::info("select dispatch receive Interrupted, errno: %d", errno);
        return 0;
    } else if (is_caught_signal_) {
        // 如果 select 执行成功，在准备处理事件前，先执行所有的信号事件
        process_signal_event();
    }
    int readable, writeable;
    // 遍历所有的 fd 对应的 event
    for (auto kv : io_event_map_) {
        readable = false;
        writeable = false;
        if (FD_ISSET(kv.first, event_read_fds_out_)) {
            readable = true;
        }
        if (FD_ISSET(kv.first, event_write_fds_out_)) {
            writeable = true;
        }
        if ((readable || writeable) && kv.second) {
            auto ev = kv.second;
            // 清除当前 event 的活跃状态，然后设置新的活跃状态
            ev->clear_event_active_status();
            if (readable && ev->is_event_type_readable()) {
                ev->enable_read_event_status_active();
            }
            if (writeable && ev->is_event_type_writeable()) {
                ev->enable_write_event_status_active();
            }
            // 如果此事件可读、可写，将它插入到活跃队列中
            if (ev->is_event_type_readable() || ev->is_event_type_writeable()) {
                // 如果这个事件是非持久性的事件，则删除这个事件
                if (!ev->is_persistent()) {
                    remove_event(ev);
                }
                push_event_active_queue(ev, 1);
            }
        }
    }
    return 0;
}
