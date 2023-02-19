// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <poll.h>
#include <map>
#include <memory>
#include "base/event_base.h"

namespace libevent_cpp {

// 采用 poll 的事件管理类
class poll_base : public event_base {
 private:
    std::map<int, struct pollfd*> fd_map_poll_;

 public:
    poll_base() = default;
    ~poll_base() = default;
    int init() override { return true; }
    int add(std::shared_ptr<io_event> ev) override;
    int remove(std::shared_ptr<io_event> ev) override;
    int recalc() override;
    int dispatch(struct timeval* tv) override;
};

}  // namespace libevent_cpp
