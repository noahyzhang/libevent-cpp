#ifndef LIBEVENT_CPP_EVENT_H
#define LIBEVENT_CPP_EVENT_H

namespace libevent_cpp {

class event {
private:  
    // bool persistent_ = false; // 是否为持久事件
    bool active_ = false;  // 当前事件是否活跃

public:  
    int priority; // 事件的优先级。值越小，优先级越高

public:
    event() = default;
    virtual ~event();

public:  
    inline void set_active() { this->active_ = true; }
    inline void set_inactive() { this->active_ = false; }
    inline bool is_active() { return this->active_; }

    // inline void set_persistent() { this->persistent_ = true; }
    // inline void set_no_persistent() { this->persistent_ = false; }
    // inline bool is_persistent() { return this->persistent_; }

    void set_priority(int priority);

};

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_EVENT_H