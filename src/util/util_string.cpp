#include "util_string.h"

bool libevent_cpp::util_string::is_equals(
    const std::string& str1, const std::string& str2) {
    if (str1.size() != str2.size()) {
        return false;
    }
    for (size_t i = 0; i < str1.size(); i++) {
        if (std::tolower(str1[i]) != std::tolower(str2[i])) {
            return false; 
        }
    }
    return true; 
}