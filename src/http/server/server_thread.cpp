#include "http/server/server_thread.h"
#include "base/epoll_base.h"
#include "util/util_linux.h"

libevent_cpp::http_server_thread::http_server_thread(http_server* server)
    : server_(server) {
    base_ = std::make_shared<epoll_base>();
    io_ev_ = create_event<io_event>(create_eventfd(), READ_ONLY);
    // TODO
    base_->register_callback(io_ev_, get_connections, io_ev_, this);
    base_->add_event(io_ev_);

    ev_sigpipe_ = create_event<signal_event>(SIGPIPE);
    // TODO
    base_->register_callback(ev_sigpipe_, ev_sigpipe_handler, ev_sigpipe_);
    base_->add_event(ev_sigpipe_);
}

libevent_cpp::http_server_thread::~http_server_thread() {
    base_->clean_io_event(ev_weaker_);
}

void libevent_cpp::http_server_thread::dispatch() {
    base_->start_dispatch();
}

