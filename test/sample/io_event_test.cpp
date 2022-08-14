#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "base/epoll_base.h"
#include "event/io_event.h"
#include "util/log/logger.h"

void fifo_read(std::shared_ptr<libevent_cpp::io_event> ev) {
    char buf[255];
    int len = read(ev->fd_, buf, sizeof(buf)-1);
    if (len < 0) {
        std::cout << "read failed" << std::endl;
        return;
    } else if (len == 0) {
        std::cout << "connection closed" << std::endl;
        return;
    } 
    buf[len] = '\0';
    std::cout << buf << std::endl;
} 

int main() {
    auto event_base = std::make_shared<libevent_cpp::epoll_base>();
    event_base->init();
    const char* fifo = "/tmp/event.fifo";

    unlink(fifo);
    if (mkfifo(fifo, 0600) < 0) {
        std::cout << "mkfifo failed" << std::endl;
        return -1;
    }
    int socket = open(fifo, O_RDWR | O_NONBLOCK, 0);
    std::cout << "socket: " << socket << std::endl;
    if (socket < 0) {
        std::cout << "open failed" << std::endl;
        return -1;
    }
    auto ev_fifo = libevent_cpp::create_event<libevent_cpp::io_event>(socket, libevent_cpp::READ_ONLY);
    event_base->register_callback(ev_fifo, fifo_read, ev_fifo);
    event_base->add_event(ev_fifo);

    event_base->start_dispatch();
}