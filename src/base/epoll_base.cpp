#include <sys/resource.h>
#include <string.h>
#include "epoll_base.h"
#include "util/log/logger.h"

bool libevent_cpp::epoll_base::init() {
    // epoll_create 的参数指定想要通过 epoll 实例来检查的文件描述符个数。
    // 该参数不是一个上限，而是告诉内核应该如何为内部数据结构划分初始大小 
    // 从 Linux 2.6.8 版本开始，此参数被忽略不用
    if ((epoll_fd_ = epoll_create(32000)) < 0) {
        logger::error("epoll_create failed");
        return false;
    }
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0 && rl.rlim_cur != RLIM_INFINITY) {
        max_events_ = rl.rlim_cur;
    }
    epoll_event_list_ = new struct epoll_event[max_events_]; 
    return true; 
}

libevent_cpp::epoll_base::~epoll_base() {
    epoll_fd_ = -1;
    max_events_ = -1;
    delete epoll_event_list_; 
    epoll_event_list_ = nullptr;
}

bool libevent_cpp::epoll_base::add(std::shared_ptr<io_event> event) {
    struct epoll_event epev = {0, {0}};
    epev.data.fd = event->fd_;
    int op = EPOLL_CTL_MOD;
    // 如果此事件是第一次添加，则使用 EPOLL_CTL_ADD 
    if (event->is_event_first_add_) {
        op = EPOLL_CTL_ADD;
        event->is_event_first_add_ = false; 
    }
    if (event->is_event_type_readable()) {
        epev.events |= EPOLLIN;
    }
    if (event->is_event_type_writeable()) {
        epev.events |= EPOLLOUT;
    }
    logger::debug("epoll_fd_: %d, op: %d, fd: %d, epoll_events: %d", epoll_fd_, op, event->fd_, epev.events); 
    if (epoll_ctl(epoll_fd_, op, event->fd_, &epev) < 0) {
        logger::error("epoll_ctl of epoll_base add failed, err: %s", strerror(errno));
        return false;
    }
    logger::debug("epoll_base::add success");
    return true; 
}

bool libevent_cpp::epoll_base::remove(std::shared_ptr<io_event> event) {
    struct epoll_event epev = {0, {0}};
    int op = EPOLL_CTL_DEL;
    // 当 EPOLL_CTL_DEL 时会忽略参数 epev 
    if (epoll_ctl(epoll_fd_, op, event->fd_, &epev) < 0) {
        logger::error("epoll_ctl of epoll_base remove failed");
        return false; 
    }
    return true; 
}

bool libevent_cpp::epoll_base::dispatch(struct timeval* tv) {
    logger::debug("epoll_base::dispatch start");
    int timeout = -1;
    if (tv) {
        // 将 tv 转换成毫秒，小于 1 毫秒的部分算作 1 毫秒
        timeout = tv->tv_sec * 1000 + (tv->tv_usec + 999) / 1000;
    }
    int res = epoll_wait(epoll_fd_, epoll_event_list_, max_events_, timeout);
    if (res < 0) {
        if (errno != EINTR) {
            logger::error("dispatch of epoll_base failed");
            return false; 
        }
        // TODO 被信号中断，处理信号
        return 0;
    }
    int what = 0;
    for (int i = 0; i < res; ++i) {
        what = epoll_event_list_[i].events;
        if (what & (EPOLLHUP | EPOLLERR)) {
            what |= (EPOLLHUP | EPOLLERR);
        }
        auto io_ev = fd_map_io_event_.at(epoll_event_list_[i].data.fd);
        if (what && io_ev) {
            // 将要处理此事件，重置其活跃状态
            io_ev->clear_event_active_status();
            if ((what & EPOLLIN) && io_ev->is_event_type_readable()) {
                // 可读事件来临，并且此事件为可读的。则设置可读为活跃状态 
                if ((what & EPOLLIN) && io_ev->is_event_type_readable()) {
                    io_ev->enable_read_event_status_active();
                }
                // 可写事件来临，并且此事件为可写的。则设置可写为活跃状态
                if ((what & EPOLLOUT) && io_ev->is_event_type_writeable()) {
                    io_ev->enable_write_event_status_active();
                }
                if (io_ev->is_read_event_active_status() || io_ev->is_write_event_active_status()) {
                    // 将此事件添加到活跃队列中
                    push_event_active_queue(io_ev);
                }
            }
        }
    }
    return true; 
}

