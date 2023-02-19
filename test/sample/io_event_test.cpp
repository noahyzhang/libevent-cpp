// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include "libevent_cpp/libevent_cpp.h"

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
    // 创建事件管理集合类
    // auto event_base = std::make_shared<libevent_cpp::select_base>();
    // auto event_base = std::make_shared<libevent_cpp::poll_base>();
    auto event_base = std::make_shared<libevent_cpp::epoll_base>();
    if (event_base->init() < 0) {
        std::cout << "epoll_base init failed" << std::endl;
        return -1;
    }

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
    // 创建 IO 事件
    auto ev_fifo = libevent_cpp::create_event<libevent_cpp::io_event>(socket, libevent_cpp::READ_ONLY);
    // 设置此事件的回调函数
    ev_fifo->set_callback(fifo_read, ev_fifo);
    // 设置此事件为持久的
    ev_fifo->set_persistent();

    // 将此 IO 事件添加到事件集合中
    event_base->add_event(ev_fifo);

    // 开始冲
    event_base->start_dispatch();
}
