#include <stdlib.h>
#include <string.h>
#include "select_base.h"
#include "util/log/logger.h"

#define MAX_SELECT_FD_SIZE 1024 

bool libevent_cpp::select_base::init() {
    event_read_fds_in_ = new fd_set();
    event_write_fds_in_ = new fd_set();
    event_read_fds_out_ = new fd_set();
    event_write_fds_out_ = new fd_set();
    return true;
}

libevent_cpp::select_base::~select_base() {
    free(event_read_fds_in_);
    event_read_fds_in_ = nullptr;
    free(event_write_fds_in_);
    event_write_fds_in_ = nullptr;
    free(event_read_fds_out_);
    event_read_fds_out_ = nullptr;
    free(event_write_fds_out_);
    event_write_fds_out_ = nullptr;
}

bool libevent_cpp::select_base::add(std::shared_ptr<io_event> ev) {
    // select 系统调用不能超过 1024 个文件描述符 
    if (ev->fd_ > MAX_SELECT_FD_SIZE) {
        logger::error("select_base add select fd > MAX_SELECT_FD_SIZE: %d", MAX_SELECT_FD_SIZE);
        return false;
    }
    // 如果事件可读，则设置读 fdset 的位 fd 
    if (ev->is_event_type_readable()) {
        FD_SET(ev->fd_, event_read_fds_in_);
    }
    // 如果事件可写，则设置写 fdset 的位 fd 
    if (ev->is_event_type_writeable()) {
        FD_SET(ev->fd_, event_write_fds_in_);
    }
    return true; 
}

bool libevent_cpp::select_base::remove(std::shared_ptr<io_event> ev) {
    // 如果事件可读，则清除读 fdset 的位 fd 
    if (!ev->is_event_type_readable()) {
        FD_CLR(ev->fd_, event_read_fds_in_);
    }
    // 如果事件可写，则清除写 fdset 的位 fd 
    if (!ev->is_event_type_writeable()) {
        FD_CLR(ev->fd_, event_write_fds_in_);
    }
    return true; 
}

bool libevent_cpp::select_base::dispatch(struct timeval* tv) {
    // 将用户设置好的 fds 拷贝，用于 select 调用
    memcpy(event_read_fds_out_, event_read_fds_in_, sizeof(fd_set));
    memcpy(event_write_fds_out_, event_write_fds_in_, sizeof(fd_set));

    int res = select(max_fd_ + 1, event_read_fds_out_, event_write_fds_out_, nullptr, tv);
    if (res < 0) {
        if (errno != EINTR) {
            logger::error("select dispatch failed, errno: %d", errno);
            return false;
        }
        logger::info("select dispatch receive Interrupted, errno: %d", errno);
        return true; 
    }
    int readable, writeable;
    // 遍历所有的 fd 对应的 event 
    for (auto kv : fd_map_io_event_) {
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
                ev->set_read_event_active_status();
            }
            if (writeable && ev->is_event_type_writeable()) {
                ev->set_write_event_active_status();
            }
            // 如果此事件可读、可写，将它插入到活跃队列中
            if (ev->is_event_type_readable() || ev->is_event_type_writeable()) {
                push_event_active_queue(ev);
            }
        }
    }
    return true; 
}