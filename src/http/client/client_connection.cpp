// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <memory>
#include "http/client/client_connection.h"
#include "util/util_logger.h"
#include "util/util_network.h"

static void libevent_cpp::read_timeout_cb(http_client_connection* conn) {
    logger::warn("client connection read timeout");
    conn->fail(HTTP_TIMEOUT);
}

static void libevent_cpp::write_timeout_cb(http_client_connection* conn) {
    logger::warn("client connection write timeout");
    conn->fail(HTTP_TIMEOUT);
}

libevent_cpp::http_client_connection::http_client_connection(
    std::shared_ptr<event_base> base,
    int fd,
    int timeout)
    : http_connection(base, fd) {
    base->register_callback(read_timer_, read_timeout_cb, this);
    base->register_callback(write_timer_, write_timeout_cb, this);
    timeout_ = timeout;
}

int libevent_cpp::http_client_connection::make_request(std::unique_ptr<http_request> req) {
    req->set_http_connection(this);
    req->set_http_request_kind(REQUEST);
    req->set_remote_host(server_addr_);
    req->set_remote_port(server_port_);
    req->make_header();
    requests_.push(req);
    if (!is_connected()) {
        if (connect() < 0) {
            return -1;
        }
    }
    start_write();
    return 0;
}

int libevent_cpp::http_client_connection::connect() {
    if (state_ == CONNECTING) {
        return 0;
    }
    reset();
    int fd = get_fd();
    if (util_network::socket_connect(fd, server_addr_, server_port_) < 0) {
        util_network::close_fd(fd);
        return -1;
    }
    add_write_event();
    state_ = CONNECTING;
    return 0;
}

void libevent_cpp::http_client_connection::do_read_done() {
    remove_read_event();
    pop_request();
    if (!requests_.empty()) {
        if (output_->get_cur_length() > 0) {
            start_write();
        } else {
            start_read();
            requests_.front()->set_http_request_kind(RESPONSE);
        }
    }
}

void libevent_cpp::http_client_connection::do_write_done() {
    auto req = current_request();
    if (!req) {
        return;
    }
    start_read();
    req->set_http_request_kind(RESPONSE);
}

void libevent_cpp::http_client_connection::fail(http_connection_error err) {
    auto req = current_request();
    if (!req) {
        return;
    }
    logger::error("request url: %s err: %d", req->uri_, err);
    pop_request();
    reset();
    if (!requests_.empty()) {
        connect();
    }
}
