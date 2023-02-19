// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <stdarg.h>
#include <string>

namespace libevent_cpp {

typedef enum {
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
} LIBEVENT_CPP_LOG_LEVEL;

#define EV_CHECK_FMT(a, b) __attribute__((format(printf, a, b)))
// TODO 在 c++ 中使用 core 掉的原因 
#define EV_NORETURN __attribute__((noreturn))

class logger {
public:
    static void log(LIBEVENT_CPP_LOG_LEVEL log_level, const char* fmt,  va_list ap);
private:
    static const char* get_log_level_str(LIBEVENT_CPP_LOG_LEVEL log_level) {
        switch (log_level) {
        case DEBUG:
            return "debug";
        case INFO:
            return "info";
        case WARNING:
            return "warn";
        case ERROR:
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
