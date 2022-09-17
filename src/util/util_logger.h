// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>

namespace libevent_cpp {

typedef enum {
    LIBEVENT_CPP_LOG_DEBUG_LEVEL = 0,
    LIBEVENT_CPP_LOG_INFO_LEVEL,
    LIBEVENT_CPP_LOG_WARN_LEVEL,
    LIBEVENT_CPP_LOG_ERROR_LEVEL,
} LIBEVENT_CPP_LOG_LEVEL;

#define EV_CHECK_FMT(a, b) __attribute__((format(printf, a, b)))
// TODO 在 c++ 中使用 core 掉的原因 
#define EV_NORETURN __attribute__((noreturn))

class logger {
 private:
    static void log(LIBEVENT_CPP_LOG_LEVEL log_level, const char* fmt,  va_list ap);
    static const char* get_log_level_str(LIBEVENT_CPP_LOG_LEVEL log_level) {
        switch (log_level) {
        case LIBEVENT_CPP_LOG_DEBUG_LEVEL:
            return "debug";
        case LIBEVENT_CPP_LOG_INFO_LEVEL:
            return "info";
        case LIBEVENT_CPP_LOG_WARN_LEVEL:
            return "warn";
        case LIBEVENT_CPP_LOG_ERROR_LEVEL:
            return "error";
        default:
            return "unkown";
        }
    }

 public:
    static void error(const char* fmt, ...) EV_CHECK_FMT(1, 2);
    static void warn(const char* fmt, ...) EV_CHECK_FMT(1, 2);
    static void info(const char* fmt, ...) EV_CHECK_FMT(1, 2);
    static void debug(const char* fmt, ...) EV_CHECK_FMT(1, 2);
};

}  // namespace libevent_cpp
