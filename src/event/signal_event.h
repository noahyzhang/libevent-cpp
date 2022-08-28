#ifndef LIBEVENT_CPP_SIGNAL_EVENT_H
#define LIBEVENT_CPP_SIGNAL_EVENT_H

#include "event.h"

namespace libevent_cpp {

class signal_event : public event {
private:  
    int signal_ = -1;

public:  
    explicit signal_event(int signal) : signal_(signal) {} 
    ~signal_event() = default;

    inline void set_signal(int signal) { signal_ = signal; }
}; 

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_SIGNAL_EVENT_H