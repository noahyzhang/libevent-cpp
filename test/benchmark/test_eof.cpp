#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include "base/epoll_base.h"
#include "base/poll_base.h"
#include "event/io_event.h"

void read_cb(std::shared_ptr<libevent_cpp::event_base> base, std::shared_ptr<libevent_cpp::io_event> ev) {
    std::cout << "func: " << __func__ << " called" << std::endl;
    char buf[256];
    int len = read(ev->fd_, buf, sizeof(buf));

    std::cout << "read: " << buf << " len: " << len << std::endl;
    if (len) {
        ev->enable_read_event_status_active();
        base->add_event(ev);
    } else {
        std::cout << "receive EOF" << std::endl;
    }
}

int main() {
    auto base = std::make_shared<libevent_cpp::poll_base>();
    base->init_priority(1);

    const char* test_str = "test string";
    int pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) < 0) {
        return -1;
    }
    write(pair[0], test_str, strlen(test_str)+1);
    shutdown(pair[0], SHUT_WR);  // shutdown write

    auto ev = libevent_cpp::create_event<libevent_cpp::io_event>(pair[1], libevent_cpp::READ_ONLY);
    base->register_callback(ev, read_cb, base, ev);

    std::cout << "pair[1]: " << pair[1] << std::endl;

    base->add_event(ev);
    base->start_dispatch();
}