#ifndef LIBEVENT_CPP_UTIL_STRING_H
#define LIBEVENT_CPP_UTIL_STRING_H

#include <string> 

namespace libevent_cpp {

class util_string {

public:
    static bool is_equals(const std::string& str1, const std::string& str2);

}; 

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_UTIL_STRING_H