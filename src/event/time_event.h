#ifndef LIBEVENT_CPP_TIME_EVENT_H
#define LIBEVENT_CPP_TIME_EVENT_H

#include <sys/time.h>
#include "event.h"

namespace libevent_cpp {

class time_event : public event {
private:  
    struct timeval timeout;

public:  
    // 构造函数，将时间清空
    time_event() {
        timerclear(&timeout);
    } 
    ~time_event() = default;

    void set_timer(int sec, int usec) {
        struct timeval now, tv;
        // 获取当前值
        gettimeofday(&now, nullptr);
        // 整理传入的参数 
        timerclear(&tv);
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        // now 和 tv 求和，做为 timeout 的值 
        timeradd(&now, &tv, &timeout); 
    }
}; 

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_TIME_EVENT_H