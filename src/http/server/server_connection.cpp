// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <utility>
#include "http/server/server_connection.h"
#include "util/util_logger.h"

namespace libevent_cpp {

http_server_connection::http_server_connection(
    std::shared_ptr<event_base> base, int fd,
    std::shared_ptr<std::map<std::string, HandleCallBack>> handle_callbacks)
    : http_connection(base, fd),
      handle_callbacks_(handle_callbacks) {
    base->register_callback(read_timer_, read_timeout_cb, this);
    base->register_callback(write_timer_, write_timeout_cb, this);
    // TODO 
    // timeout 
}

void http_server_connection::read_timeout_cb(http_server_connection* conn) {
    logger::warn("server connection read timeout, client addr: %s, port: %d",
        conn->get_client_address(), conn->get_client_port());
    conn->fail(HTTP_TIMEOUT);
}

void http_server_connection::write_timeout_cb(http_server_connection* conn) {
    logger::warn("server connection write timeout, client addr: %s, port: %d",
        conn->get_client_address(), conn->get_client_port());
    conn->fail(HTTP_TIMEOUT);
}

int http_server_connection::create_request() {
    auto req = get_empty_request();
    // TODO flag
    req->set_http_request_kind(REQUEST);
    req->set_remote_host(client_address_);
    req->set_remote_port(client_port_);
    requests_.push(std::move(req));
    start_read();
    return 0;
}

void http_server_connection::handle_request(http_request* req) {
    // 先判断请求的 URI 是否正确
    if (req->get_uri().empty()) {
        req->send_error_reply(HTTP_BADREQUEST, "Bad Request");
        logger::error("request uri is empty, handle Bad Requst");
        return;
    }
    logger::info("handle uri: %s", req->get_uri().c_str());

    req->set_uri(util_string::string_from_utf8(req->get_uri()));
    // 对请求进行分割
    size_t offset = req->get_uri().find('?');
    if (offset != std::string::npos) {
        // TODO req->query 
    }
    // 如果已经存在，则更新
    if (server_->handle_callbacks.count(req->get_uri()) > 0) {
        server_->handle_callbacks.at(req->get_uri())(req);
        return;
    }
    // TODO 
    for (const auto& kv : server_->handle_callbacks) {
    }
}

void http_server_connection::fail(http_connection_error err) {
    logger::warn("server connection fail on err: %d, state: %d", err, state_);
    /**
     * 对于请求，有两类不同的错误
     * 1. 网络传输层的错误，比如超时，我们直接丢弃这条连接
     * 2. HTTP 应用层的错误，我们应该在释放这个连接前回传一个操作
     */
    switch (err) {
    case HTTP_TIMEOUT:
        close(1);
        return;
    case HTTP_EOF:
        close(0);
        return;
    case HTTP_INVALID_HEADER:
    default:
        auto req = get_current_request();
        if (!req) {
            return;
        }
        if (!req->get_uri().empty()) {
            req->set_uri("");
        }
        if (req->get_cb()) {
            req->get_cb()(req);
        }
        handle_request(req);
        return;
    }
}

void http_server_connection::do_read_done() {
    auto req = get_current_request();
    if (!req) {
        return;
    }
    if (req->is_handled()) {
        fail(HTTP_EOF);
        return;
    }
    start_write();
    handle_request(req);
    req->set_handled();
}

void http_server_connection::do_write_done() {
    auto req = get_current_request();
    if (!req) {
        return;
    }
    // TODO chunk
    bool need_close = req->is_connection_close();
    pop_request();
    if (need_close) {
        close(0);
        return;
    }
    if (create_request() < 0) {
        close(1);
    }
}

}  // namespace libevent_cpp
