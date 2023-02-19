// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include "libevent_cpp/libevent_cpp.h"

void signal_cb() {
    std::cout << "signal call back" << std::endl;
}

int main() {
    // 创建事件管理集合类
    auto event_base = std::make_shared<libevent_cpp::select_base>();
    if (event_base->init() < 0) {
        std::cout << "init event_base failed" << std::endl;
        return -1;
    }

    // 创建信号事件
    auto event = libevent_cpp::create_event<libevent_cpp::signal_event>(SIGINT);
    event->set_callback(signal_cb);
    event->set_persistent();

    // 将信号事件添加到事件管理集合中
    event_base->add_event(event);

    // run
    event_base->start_dispatch();
}
