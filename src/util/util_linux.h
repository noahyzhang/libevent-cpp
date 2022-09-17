// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <sys/eventfd.h>
#include "util/util_logger.h"

namespace libevent_cpp {

class util_linux {
 public:
    static inline int create_eventfd() {
        int evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evfd < 0) {
            logger::error("linux eventfd error");
            abort();
        }
        return evfd;
    }
};

}  // namespace libevent_cpp
