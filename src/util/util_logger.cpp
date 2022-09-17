// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <stdarg.h>
#include <iostream>
#include "util/util_logger.h"

void libevent_cpp::logger::log(LIBEVENT_CPP_LOG_LEVEL log_level, const char* fmt, va_list ap) {
    char buf[1024];
    size_t bufLen = sizeof(buf);
    if (fmt != NULL) {
        vsnprintf(buf, bufLen, fmt, ap);
        buf[bufLen-1] = '\0';
    } else {
        buf[0] = '\0';
    }
    fprintf(stderr, "[Libevent-cpp %s] %s \n", get_log_level_str(log_level), buf);
}

void libevent_cpp::logger::error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LIBEVENT_CPP_LOG_ERROR_LEVEL, fmt, ap);
    va_end(ap);
}

void libevent_cpp::logger::warn(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LIBEVENT_CPP_LOG_WARN_LEVEL, fmt, ap);
    va_end(ap);
}

void libevent_cpp::logger::info(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LIBEVENT_CPP_LOG_INFO_LEVEL, fmt, ap);
    va_end(ap);
}

void libevent_cpp::logger::debug(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    log(LIBEVENT_CPP_LOG_DEBUG_LEVEL, fmt, ap);
    va_end(ap);
}
