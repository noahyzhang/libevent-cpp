#include "base/epoll_base.h"
#include "event/time_event.h"
#include <iostream>

#define EVENT_COUNT 1000

void timer_cb(std::shared_ptr<libevent_cpp::event_base> base, std::shared_ptr<libevent_cpp::time_event> ev) {
    std::cout << "id: " << ev->event_id_ << ", timeout: " <<
        ev->get_timeout().tv_sec << "s, " << ev->get_timeout().tv_usec << "us." << std::endl;
    base->remove_event(ev);
}

int main() {
    auto base = std::make_shared<libevent_cpp::epoll_base>();
    // 初始化优先级
    base->init_priority(1);

    for (size_t i = 0; i < EVENT_COUNT; i++) {
        auto ev = libevent_cpp::create_event<libevent_cpp::time_event>();
        ev->set_timer(random() % 10, 0);
        base->register_callback(ev, timer_cb, base, ev);
        base->add_event(ev);
    }
    // 开始调度
    base->start_dispatch();
}