#ifndef LIBEVENT_CPP_SERVER_THREAD_H
#define LIBEVENT_CPP_SERVER_THREAD_H

#include "http/server/server.h"
#include "event/signal_event.h"

namespace libevent_cpp {
    
class http_server_thread {

public:  
    http_server_thread(http_server* server);

private:
    std::shared_ptr<event_base> base_ = nullptr;
    http_server* server_;
    std::shared_ptr<io_event> ev_weaker_;
    std::shared_ptr<signal_event> 

}; 

} // namespace libevent_cpp 


#endif // LIBEVENT_CPP_SERVER_THREAD_H 