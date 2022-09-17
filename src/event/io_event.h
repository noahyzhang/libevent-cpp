// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <unistd.h>
#include "event/event.h"

namespace libevent_cpp {

enum EVENT_TYPE {
    NONE = 0,
    READ_ONLY,
    WRITE_ONLY,
    RDWR,
};

class io_event : public event {
 private:
    bool event_type_read_ = false;  // 事件类型：读事件
    bool event_type_write_ = false;  // 事件类型：写事件
    bool read_event_status_active_ = false;  // 读事件的活跃状态
    bool write_event_status_active_ = false;  // 写事件的活跃状态

 public:
    int fd_ = -1;  // 文件描述符
    bool is_event_first_add_ = true;  // 事件如果第一次添加，则为 add；后续为 mod
    int err = -1;

 public:
    io_event() = delete;
    io_event(int fd, EVENT_TYPE type) : fd_(fd) { set_event_type(type); }
    ~io_event()  {
        if (fd_ > 0) {
            close(fd_);
        }
    }

    inline void set_fd(size_t fd) { fd_ = fd; }

    inline void set_event_type(EVENT_TYPE type) {
        switch (type) {
        case READ_ONLY:
            event_type_read_ = true;
            break;
        case WRITE_ONLY:
            event_type_write_ = true;
            break;
        case RDWR:
            event_type_read_ = event_type_write_ = true;
            break;
        default:
            break;
        }
    }
    // 清除事件的活跃状态
    inline void clear_event_active_status() { read_event_status_active_ = write_event_status_active_ = false; }
    // 设置事件的活跃状态为可读
    inline void enable_read_event_status_active() { read_event_status_active_ = true; }
    // 设置事件的活跃状态为可写
    inline void enable_write_event_status_active() { write_event_status_active_ = true; }
    // 清除可读时间的活跃状态
    inline void disable_read_event_status_active() { read_event_status_active_ = false; }
    // 清除可写事件的活跃状态
    inline void disable_write_event_status_active() { write_event_status_active_ = false; }
    // 此事件的读活跃状态
    inline bool is_read_event_active_status() const { return read_event_status_active_; }
    // 此事件的写活跃状态
    inline bool is_write_event_active_status() const { return write_event_status_active_; }

    // 是否为可读事件
    inline bool is_event_type_readable() const { return event_type_read_; }
    // 是否为可写事件
    inline bool is_event_type_writeable() const { return event_type_write_; }
    // 如果此事件既不可读，也不可写。则认为这个事件可被删除
    inline bool is_event_type_removeable() const { return !event_type_read_ && !event_type_write_; }
};

}  // namespace libevent_cpp
