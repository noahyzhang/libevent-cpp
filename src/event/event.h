// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <memory>
#include <utility>

namespace libevent_cpp {

class event {
 private:
    // bool persistent_ = false; // 是否为持久事件
    bool active_ = false;  // 当前事件是否活跃
    static int internal_event_id_;  // 内部的 event_id，负责给 event 一个 id

 public:
    int event_id_;  // 用来标识事件
    int priority_;  // 事件的优先级。值越小，优先级越高

 public:
    event();
    virtual ~event() {}

 public:
    inline void set_active_status() { this->active_ = true; }
    inline void clear_active_status() { this->active_ = false; }
    inline bool get_active_status() { return this->active_; }

    // inline void set_persistent() { this->persistent_ = true; }
    // inline void set_no_persistent() { this->persistent_ = false; }
    // inline bool is_persistent() { return this->persistent_; }

    void set_priority(int priority);
};

template <typename T, typename... Rest>
std::shared_ptr<T> create_event(Rest &&... rest) {
    return std::make_shared<T>(std::forward<Rest>(rest)...);
}

}  // namespace libevent_cpp

