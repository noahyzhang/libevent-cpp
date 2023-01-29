// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <memory>
#include <utility>

namespace libevent_cpp {

// 事件的基类
class event {
 public:
    event();
    virtual ~event() {}

 public:
    // 设置事件的状态为活跃
    inline void set_active_status() { this->active_ = true; }
    // 清楚事件的活跃状态
    inline void set_inactive_status() { this->active_ = false; }
    // 获取事件的活跃状态
    inline bool is_active_status() { return this->active_; }

    // 设置事件的持久性
    inline void set_persistent() { this->persistent_ = true; }
    // 设置事件不是持久的
    inline void set_no_persistent() { this->persistent_ = false; }
    // 获取事件的是否是持久的
    inline bool is_persistent() const { return this->persistent_; }

    // 设置事件优先级
    void set_priority(int priority);

 private:
    // 是否为持久事件
    bool persistent_ = false;
    // 当前事件是否活跃
    bool active_ = false;
    // 内部的 event_id，负责给 event 一个 id
    static int internal_event_id_;

 public:
    // 用来标识事件
    int event_id_;
    // 事件的优先级。值越小，优先级越高
    int priority_;
    // 调用次数
    size_t call_num_ = 0;
};

// 创建一个事件，其中 T 为事件类型，Rest 为参数
template <typename T, typename... Rest>
std::shared_ptr<T> create_event(Rest &&... rest) {
    return std::make_shared<T>(std::forward<Rest>(rest)...);
}

}  // namespace libevent_cpp

