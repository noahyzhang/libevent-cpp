// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include "base/poll_base.h"
#include "event/time_event.h"

void timeout_cb(std::shared_ptr<libevent_cpp::time_event> ev) {
    time_t new_time = time(nullptr);

    std::cout << __func__ << ": called at " << new_time << std::endl;

    ev->set_timer(3, 0);

    // ev->get_base()->add_event(ev);
}

int main() {
    auto event_base = std::make_shared<libevent_cpp::poll_base>();
    event_base->init();

    auto event = libevent_cpp::create_event<libevent_cpp::time_event>();
    event_base->register_callback(event, timeout_cb, event);
    event_base->add_event(event);

    event_base->start_dispatch();
}
