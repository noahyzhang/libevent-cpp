// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include "libevent_cpp/libevent_cpp.h"

void timeout_cb(
    std::shared_ptr<libevent_cpp::poll_base> event_base,
    std::shared_ptr<libevent_cpp::time_event> ev) {
    time_t new_time = time(nullptr);

    std::cout << __func__ << ": called at " << new_time << std::endl;

    ev->set_timer(1, 0);
    event_base->add_event(ev);
}

int main() {
    // 创建事件管理集合
    auto event_base = std::make_shared<libevent_cpp::poll_base>();
    event_base->init();

    // 创建定时事件
    auto event = libevent_cpp::create_event<libevent_cpp::time_event>();
    event->set_callback(timeout_cb, event_base, event);

    // 将此定时事件添加到事件管理集合中
    event_base->add_event(event);

    // run
    event_base->start_dispatch();
}
