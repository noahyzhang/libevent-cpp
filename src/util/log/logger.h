#ifndef LIBEVENT_CPP_LOGGER_H
#define LIBEVENT_CPP_LOGGER_H

namespace libevent_cpp {

#define EV_CHECK_FMT(a,b) __attribute__((format(printf, a, b))) 
#define EV_NORETURN __attribute__((noreturn)) 

class logger {

public:  
    static void error(const char* fmt, ...) EV_CHECK_FMT(1, 2) EV_NORETURN; 
    static void warn(const char* fmt, ...) EV_CHECK_FMT(1, 2) EV_NORETURN;
    static void info(const char* fmt, ...) EV_CHECK_FMT(1, 2) EV_NORETURN;
    static void debug(const char* fmt, ...) EV_CHECK_FMT(1, 2) EV_NORETURN; 

};

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_LOGGER_H