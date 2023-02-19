// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <sys/time.h>
#include "event/event.h"

namespace libevent_cpp {

// 定时事件
class time_event : public event {
 public:
    // 构造函数，将时间清空
    time_event() {
        timerclear(&timeout_);
    }
    ~time_event() = default;

 public:
    // 设置定时事件的超时事件
    void set_timer(int sec, int usec) {
        struct timeval now, tv;
        // 获取当前值
        gettimeofday(&now, nullptr);
        // 整理传入的参数
        timerclear(&tv);
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        // now 和 tv 求和，做为 timeout 的值
        timeradd(&now, &tv, &timeout_);
    }
    // 获取定时时间
    inline timeval get_timeout() const {
        return timeout_;
    }

 private:
    // 超时事件
    struct timeval timeout_;
};

}  // namespace libevent_cpp
