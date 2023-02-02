// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <memory>

namespace libevent_cpp {

class thread_task_handler : public std::enable_shared_from_this<thread_task_handler> {
public:
    std::shared_ptr<thread_task_handler> get_shared_this_ptr() {
        return thread_task_handler::shared_from_this();
    }

};

}  // namespace libevent_cpp
