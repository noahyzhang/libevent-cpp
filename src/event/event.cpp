// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include "event/event.h"

int libevent_cpp::event::internal_event_id_ = 0;

libevent_cpp::event::event() {
    callback_ = std::make_shared<std::function<void()>>();
    event_id_ = internal_event_id_++;
}

void libevent_cpp::event::set_priority(int priority) {
    // 处于活跃状态的 event 不能设置优先级
    if (is_active_status()) {
        return;
    }
    // 设置优先级，这里没有做判断。在使用时需要做判断
    priority_ = priority;
}
