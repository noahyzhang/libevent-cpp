// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <list>
#include <queue>
#include <memory>
#include "http/server/server.h"
#include "event/signal_event.h"
#include "http/server/server_connection.h"

namespace libevent_cpp {

class http_server_thread {
 public:
    explicit http_server_thread(http_server* server);
    ~http_server_thread();

    void dispatch();
    void set_terminate();

 private:
    std::unique_ptr<http_server_connection> get_empty_connection();
    static void get_connections(std::shared_ptr<io_event> ev, http_server_thread* thread);
    static void ev_sigpipe_handler(std::shared_ptr<signal_event> ev);

 private:
    std::shared_ptr<event_base> base_ = nullptr;
    http_server* server_;
    std::shared_ptr<io_event> io_ev_;
    std::shared_ptr<signal_event> ev_sigpipe_;
    std::list<std::unique_ptr<http_server_connection>> connection_list_;
    std::queue<std::unique_ptr<http_server_connection>> empty_queue_;
};

}  // namespace libevent_cpp

