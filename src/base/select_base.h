// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <sys/select.h>
#include <memory>
#include "base/event_base.h"

namespace libevent_cpp {

class select_base : public event_base {
 private:
    // 用户修改的 fds 和传入内核的 fds 分开，为了线程安全
    // 用于用户修改监听的 fd
    fd_set* event_read_fds_in_ = nullptr;
    fd_set* event_write_fds_in_ = nullptr;
    // 用于 select dispatch 调用，由内核填充内部
    fd_set* event_read_fds_out_ = nullptr;
    fd_set* event_write_fds_out_ = nullptr;

    int max_fd_;  // 最大的文件描述符
 public:
    select_base() = default;
    ~select_base();
    bool init() override;
    bool add(std::shared_ptr<io_event> ev) override;
    bool remove(std::shared_ptr<io_event> ev) override;
    bool dispatch(struct timeval* tv) override;
};

}  // namespace libevent_cpp
