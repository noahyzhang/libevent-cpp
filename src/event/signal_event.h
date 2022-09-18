// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include "event/event.h"

namespace libevent_cpp {

class signal_event : public event {
 private:
    int signal_ = -1;

 public:
    explicit signal_event(int signal) : signal_(signal) {}
    ~signal_event() = default;

    inline void set_signal(int signal) { signal_ = signal; }
    inline int get_signal() const { return signal_; }
};

}  // namespace libevent_cpp
