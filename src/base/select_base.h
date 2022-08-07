#ifndef LIBEVENT_CPP_SELECT_BASE_H
#define LIBEVENT_CPP_SELECT_BASE_H

#include <sys/select.h>
#include "event_base.h"

namespace libevent_cpp {

class select_base : public event_base {
private:  
    fd_set* event_read_set_in = nullptr;
    fd_set* event_write_set_in = nullptr;
    
    fd_set* event_read_set_out = nullptr;
    fd_set* event_write_set_out = nullptr;
public:  
    select_base() = default;
    ~select_base();
    bool init() override;
    bool add(std::shared_ptr<io_event> ev) override;
    bool remove(std::shared_ptr<io_event> ev) override;
    bool dispatch(struct timeval* tv) override; 
};

} // namespace libevent_cpp

#endif // LIBEVENT_CPP_SELECT_BASE_H