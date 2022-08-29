#include "poll_base.h"
#include "util/log/logger.h"

bool libevent_cpp::poll_base::add(std::shared_ptr<io_event> ev) {
    struct pollfd* pfd = fd_map_poll_[ev->fd_];
    if (!pfd) {
        pfd = new struct pollfd();
        pfd->fd = ev->fd_;
        pfd->events = 0;
        pfd->revents = 0;
        fd_map_poll_[ev->fd_] = pfd;
    }
    if (ev->is_event_type_readable()) {
        pfd->events |= POLLIN;
    }
    if (ev->is_event_type_writeable()) {
        pfd->events |= POLLOUT;
    }
    return true; 
}

bool libevent_cpp::poll_base::remove(std::shared_ptr<io_event> ev) {
    delete fd_map_poll_[ev->fd_];
    fd_map_poll_.erase(ev->fd_);
    return true; 
}

bool libevent_cpp::poll_base::dispatch(struct timeval* tv) {
    int timeout = -1;
    if (tv) {
        // timeout 单位为毫秒。小于 1 毫秒的数值当作 1 毫秒
        timeout = tv->tv_sec * 1000 + (tv->tv_usec + 999) / 1000;
    }
    int nfds = fd_map_poll_.size();
    struct pollfd fds[nfds];
    int i = 0;
    for (const auto kv : fd_map_poll_) {
        fds[i++] = *kv.second;
    }
    int res = poll(fds, nfds, timeout);
    if (res < 0) {
        if (errno != EINTR) {
            logger::error("poll_base::dispatch poll err");
            return false; 
        }
        // TODO 其他信号来了，需要处理 
        return true;
    }
    if (res == 0) return true; // 如果没有就绪的文件描述符，直接返回
    int what = 0;
    for (int i = 0; i < nfds; i++) {
        what = fds[i].revents;
        auto io_ev = fd_map_io_event_[fds[i].fd];
        if (what && io_ev) {
            io_ev->clear_event_active_status();
            if (what & (POLLHUP | POLLERR)) {
                what |= POLLIN | POLLOUT;
            }
            if ((what & POLLIN) && io_ev->is_event_type_readable()) {
                io_ev->enable_read_event_status_active();
            }
            if ((what & POLLOUT) && io_ev->is_event_type_writeable()) {
                io_ev->enable_write_event_status_active();
            }
            if (io_ev->is_read_event_active_status() || io_ev->is_write_event_active_status()) {
                push_event_active_queue(io_ev); 
            }
        }
    }
    return true; 
}