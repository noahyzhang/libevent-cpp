#ifndef LIBEVENT_CPP_EPOLL_BASE_H
#define LIBEVENT_CPP_EPOLL_BASE_H

#include <sys/epoll.h>
#include "event_base.h"

namespace libevent_cpp {

class epoll_base : public event_base {
private:  
    int epoll_fd_; // epoll 实例的文件描述符
    struct epoll_event* epoll_event_list_;  // epoll 的事件列表
    int max_events_; // epoll 的事件列表个数

public: 
    epoll_base() = default;
    ~epoll_base();

    /**
     * @brief 初始化类，之所以不放在构造函数中，是因为初始化过程如果出错，对象的状态未知 
     * 
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool init() override; 
    bool add(std::shared_ptr<io_event> event) override; 
    bool remove(std::shared_ptr<io_event> event) override;
    bool dispatch(struct timeval* tv) override;
};

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_EPOLL_BASE_H