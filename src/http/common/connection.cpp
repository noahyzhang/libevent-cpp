// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <memory>
#include <utility>
#include "http/common/request.h"
#include "http/common/connection.h"
#include "util/util_logger.h"

libevent_cpp::http_connection::http_connection(
    std::shared_ptr<event_base> base, int fd)
    : buffer_event(base, fd)  {
    state_ = DISCONNECTED;
    register_read_cb(handler_read, this);
    register_eof_cb(handler_eof, this);
    register_write_cb(handler_write, this);
    register_error_cb(handler_error, this);

    read_timer_ = create_event<time_event>();
    write_timer_ = create_event<time_event>();
}

void libevent_cpp::http_connection::reset() {

}

void libevent_cpp::http_connection::close(int op) {

}

void libevent_cpp::http_connection::start_read() {
    state_ = READING_FIRSTLINE;
    if (get_input_buf_length() > 0) {
        read_http();
    } else {
        add_read_and_timer();
    }
}

void libevent_cpp::http_connection::start_write() {
    if (get_output_buf_length() <= 0) {
        return;
    }
    state_ = WRITING;
    add_write_and_timer();
}

void libevent_cpp::http_connection::add_read_and_timer() {
    add_read_event();
    if (timeout_ > 0) {
        read_timer_->set_timer(timeout_, 0);
        get_event_base()->add_event(read_timer_);
    }
}

void libevent_cpp::http_connection::add_write_and_timer() {
    add_write_event();
    if (timeout_ > 0) {
        write_timer_->set_timer(timeout_, 0);
        get_event_base()->add_event(write_timer_);
    }
}

void libevent_cpp::http_connection::remove_read_and_timer() {
    get_event_base()->remove_event(read_timer_);
}

void libevent_cpp::http_connection::remove_write_and_timer() {
    get_event_base()->remove_event(write_timer_);
}

inline libevent_cpp::http_request* libevent_cpp::http_connection::current_request() {
    if (requests_.empty()) {
        logger::warn("no request");
        // TODO 
        close(1);
        return nullptr;
    }
    return requests_.front().get();
}

void libevent_cpp::http_connection::pop_request() {
    if (requests_.empty()) {
        return;
    }
    auto req = std::move(requests_.front());
    requests_.pop();
    if (req->get_cb()) {
        req->get_cb()(req.get());
    }
    req->reset();
    empty_queue_.push(std::move(req));
}

inline std::unique_ptr<libevent_cpp::http_request> libevent_cpp::http_connection::get_empty_request() {
    if (empty_queue_.empty()) {
        return std::unique_ptr<http_request>(new http_request(this));
    }
    auto req = std::move(empty_queue_.front());
    empty_queue_.pop();
    return req;
}

void libevent_cpp::http_connection::read_http() {
    if (is_closed() || requests_.empty()) {
        return;
    }
    switch (state_) {
    case READING_FIRSTLINE:
        read_firstline();
        break;
    case READING_HEADERS:
        read_header();
        break;
    case READING_BODY:
        read_body();
        break;
    case READING_TRAILER:
        read_trailer();
        break;
    case DISCONNECTED:
    case CONNECTING:
    case IDLE:
    case WRITING:
    default:
        logger::error("invalid connection stte: %d", state_);
        break;
    }
}

void read_firstline() {

}

void read_header() {

}

void get_body() {

}

void read_body() {

}

void read_trailer() {

}

void libevent_cpp::http_connection::handler_read(http_connection* conn) {
    conn->remove_read_and_timer();
    conn->read_http();
}

void libevent_cpp::http_connection::handler_eof(http_connection* conn) {
    if (conn->get_output_buf_length() > 0) {
        conn->start_write();
    }
}

void libevent_cpp::http_connection::handler_write(http_connection* conn) {
    conn->remove_write_and_timer();
    if (conn->get_output_buf_length() > 0) {
        conn->add_write_and_timer();
    } else {
        conn->remove_write_event();
        conn->do_write_done();
    }
}

void libevent_cpp::http_connection::handler_error(http_connection* conn) {
    if (errno == EPIPE || errno == ECONNRESET) {
        conn->
    }
}
