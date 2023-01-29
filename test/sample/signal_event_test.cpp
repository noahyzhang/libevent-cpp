// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include "base/select_base.h"
#include "event/signal_event.h"

void signal_cb() {
    std::cout << "signal call back" << std::endl;
}

int main() {
    auto event_base = std::make_shared<libevent_cpp::select_base>();
    event_base->init();

    auto event = libevent_cpp::create_event<libevent_cpp::signal_event>(SIGINT);
    event_base->register_callback(event, signal_cb);
    event_base->add_event(event);

    event_base->start_dispatch();
}
