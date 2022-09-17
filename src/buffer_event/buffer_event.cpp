// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <memory>
#include "buffer_event/buffer_event.h"

libevent_cpp::buffer_event::buffer_event(std::shared_ptr<event_base> base, int fd) : base_(base) {
    input_ = std::unique_ptr<buffer>(new buffer);
    output_ = std::unique_ptr<buffer>(new buffer);
    ev_ = std::make_shared<io_event>(fd, NONE);
    base->register_callback(ev_, io_callback, this);
}

int libevent_cpp::buffer_event::write(void* data, size_t size) {
    int res = output_->push_back(data, size);
    if (res < 0) {
        return -1;
    }
    if (size > 0) {
        add_write_event();
    }
    return res;
}

int libevent_cpp::buffer_event::read(void* data, size_t size) {
    return input_->pop_front(data, size);
}

void libevent_cpp::buffer_event::add_read_event() {
    ev_->enable_read_event_status_active();
    get_event_base()->add_event(ev_);
}

void libevent_cpp::buffer_event::add_write_event() {
    ev_->enable_write_event_status_active();
    get_event_base()->add_event(ev_);
}

void libevent_cpp::buffer_event::remove_read_event() {
    ev_->disable_read_event_status_active();
    get_event_base()->remove_event(ev_);
}

void libevent_cpp::buffer_event::remove_write_event() {
    ev_->disable_write_event_status_active();
    get_event_base()->remove_event(ev_);
}

void libevent_cpp::buffer_event::io_callback(buffer_event* bev) {
    int res = 0;
    auto ev = bev->ev_;
    if (ev->is_read_event_active_status()) {
        res = bev->read_input_buffer();
        if (res > 0) {
            bev->add_read_event();
            if (bev->read_cb_) {
                (*bev->read_cb_)();
            }
        } else {
            if (res == 0) {
                ev->err = EOF;
                if (bev->eof_cb_) {
                    (*bev->eof_cb_)();
                }
            } else {
                ev->err = errno;
                // 对于资源不可用或者信号中断的错误，可以去读
                if (errno == EAGAIN || errno == EINTR) {
                    bev->add_read_event();
                } else if (bev->error_cb_) {
                    (*bev->error_cb_)();
                }
            }
        }
    }

    if (ev->is_write_event_active_status() && bev->get_output_buf_length() > 0) {
        res = bev->write_output_buffer();
        if (res > 0) {
            bev->add_write_event();
        } else {
            if (res == 0) {
                ev->err = EOF;
            } else {
                ev->err = errno;
                if (errno == EAGAIN || errno == EINTR || errno == EINPROGRESS) {
                    bev->add_write_event();
                } else if (bev->error_cb_) {
                    (*bev->error_cb_)();
                }
            }
        }
        if (bev->write_cb_) {
            (*bev->write_cb_)();
        }
    }
}
