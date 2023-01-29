// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <sys/select.h>
#include <memory>
#include "base/event_base.h"

namespace libevent_cpp {

// 采用 select 的事件管理类
class select_base : public event_base {
 private:
    // 用户修改的 fds 和传入内核的 fds 分开，为了线程安全
    // 用于用户修改监听的 fd
    fd_set* event_read_fds_in_ = nullptr;
    fd_set* event_write_fds_in_ = nullptr;
    // 用于 select dispatch 调用，由内核填充内部
    fd_set* event_read_fds_out_ = nullptr;
    fd_set* event_write_fds_out_ = nullptr;
    // 最大的文件描述符
    int max_fd_;
 public:
    select_base() = default;
    ~select_base();
    int init() override;
    int add(std::shared_ptr<io_event> ev) override;
    int remove(std::shared_ptr<io_event> ev) override;
    int dispatch(struct timeval* tv) override;
};

}  // namespace libevent_cpp
