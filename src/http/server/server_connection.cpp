#include "http/server/server_connection.h"
#include "util/util_logger.h"

void libevent_cpp::read_timeout_cb(http_server_connection* conn) {
    logger::warn("server connection read timeout, client addr: %s, port: %d", 
        conn->get_client_address(), conn->get_client_port());
    conn->fail(HTTP_TIMEOUT);
}

void libevent_cpp::write_timeout_cb(http_server_connection* conn) {
    logger::warn("server connection write timeout, client addr: %s, port: %d", 
        conn->get_client_address(), conn->get_client_port());
    conn->fail(HTTP_TIMEOUT);
}

libevent_cpp::http_server_connection::http_server_connection(
    std::shared_ptr<event_base> base, int fd, http_server* server)
    : http_connection(base, fd), server_(server) {
    base->register_callback(read_timer_, read_timeout_cb, this);
    base->register_callback(write_timer_, write_timeout_cb, this);
    // TODO 
    // timeout 
}

int libevent_cpp::http_server_connection::create_request() {
    auto req = get_empty_request();
    // TODO flag
    req->set_http_request_kind(REQUEST);
    req->set_remote_host(client_address_);
    req->set_remote_port(client_port_);
    requests_.push(std::move(req));
    start_read();
    return true;
}

void libevent_cpp::http_server_connection::handle_request(http_request* req) {

}

void libevent_cpp::http_server_connection::fail(http_connection_error err) {
    logger::warn("server connection fail on err: %d, state: %d", err, state_); 
    switch (err)
    {
    case HTTP_TIMEOUT:
        close(1);
        return;
    case HTTP_EOF:
        close(0);
        return;
    case HTTP_INVALID_HEADER:
    default:
        auto req = current_request();
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