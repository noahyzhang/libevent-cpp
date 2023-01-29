// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include "event/event.h"

namespace libevent_cpp {

// 信号事件
class signal_event : public event {
 public:
    explicit signal_event(int signal) : signal_(signal) {}
    ~signal_event() = default;

 public:
    // 设置事件的信号值
    inline void set_signal(int signal) { signal_ = signal; }
    // 获取事件的信号值
    inline int get_signal() const { return signal_; }

 private:
    // 信号值
    int signal_ = -1;
};

}  // namespace libevent_cpp
