#ifndef LIBEVENT_CPP_EVENT_BASE_H
#define LIBEVENT_CPP_EVENT_BASE_H

#include <vector>
#include <queue>
#include <map> 
#include <memory>
#include <functional>
#include <time.h>
#include "event/event.h"
#include "event/io_event.h"

namespace libevent_cpp {

using Callback = std::function<void()>;

class event_base {
private:  
    std::vector<std::queue<std::shared_ptr<event>>> active_event_queues_;  // 存放活跃事件的队列
    std::map<int, std::shared_ptr<Callback>> callback_func_map_; // 存放回调函数的 map 

protected:  
    std::map<int, std::shared_ptr<io_event>> fd_map_io_event_;

public:  
    virtual bool init() { return true; } 
    virtual bool add(std::shared_ptr<io_event>) { return true; }
    virtual bool remove(std::shared_ptr<io_event>) { return true; }
    virtual bool dispatch(struct timeval*) { return true; }
    
public:  
    // 添加事件
    bool add_event(const std::shared_ptr<event>& ev);
    // 移除事件
    bool remove_event(const std::shared_ptr<event>& ev);
    // 将事件添加到活跃队列中
    void push_event_active_queue(std::shared_ptr<event> ev); 

};

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_EVENT_BASE_H