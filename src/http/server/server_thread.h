#ifndef LIBEVENT_CPP_SERVER_THREAD_H
#define LIBEVENT_CPP_SERVER_THREAD_H

#include <list>
#include <queue>

#include "http/server/server.h"
#include "event/signal_event.h"
#include "http/server/server_connection.h"

namespace libevent_cpp {
    
class http_server_thread {

public:  
    http_server_thread(http_server* server);
    ~http_server_thread();

    void dispatch();

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

} // namespace libevent_cpp 


#endif // LIBEVENT_CPP_SERVER_THREAD_H 