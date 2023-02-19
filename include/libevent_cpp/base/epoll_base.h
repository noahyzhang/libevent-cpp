// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <sys/epoll.h>
#include <memory>
#include "base/event_base.h"

namespace libevent_cpp {

// 采用 epoll 的事件管理类
class epoll_base : public event_base {
 private:
    // epoll 实例的文件描述符
    int epoll_fd_;
    // epoll 的事件列表
    struct epoll_event* epoll_event_list_;
    // epoll 的事件列表个数
    int max_events_;

 public:
    epoll_base() = default;
    ~epoll_base();
    int init() override;
    int add(std::shared_ptr<io_event> event) override;
    int remove(std::shared_ptr<io_event> event) override;
    int recalc() override;
    int dispatch(struct timeval* tv) override;
};

}  // namespace libevent_cpp
