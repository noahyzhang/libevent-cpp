#ifndef LIBEVENT_CPP_POLL_BASE_H
#define LIBEVENT_CPP_POLL_BASE_H

#include <map>
#include <poll.h>
#include "event_base.h"

namespace libevent_cpp {

class poll_base : public event_base {
private:  
    std::map<int, struct pollfd*> fd_map_poll_; 
public:  
    poll_base() = default;
    ~poll_base() = default;
    bool init() override { return true; }
    bool add(std::shared_ptr<io_event> ev) override;
    bool remove(std::shared_ptr<io_event> ev) override;
    bool dispatch(struct timeval* tv) override; 
}; 

} // namespace libevent_cpp

#endif // LIBEVENT_CPP_POLL_BASE_H