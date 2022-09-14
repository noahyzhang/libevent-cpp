#include <sstream>
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

std::string libevent_cpp::util_string::string_from_utf8(const std::string& in) {
    std::string result;
    std::stringstream ss;
    size_t i = 0, len = in.length();
    bool flag = false;
    while (i < len) {
        char ch = in[i++];
        if (ch == '%') {
            flag = true;
            ss << static_cast<char>((16 * hex_to_int(in[i++])) + hex_to_int(in[i++]));
        } else {
            if (flag) {
                result += ss.str();
                std::stringstream().swap(ss);
            }
            result += ch;
            flag = false;
        }
    }
    if (flag) {
        result += ss.str();
    }
    return result; 
}

int libevent_cpp::util_string::hex_to_int(char ch) {
    if (ch >= '0' && ch <= '9') {
        return (ch - 48);
    } else if (ch >= 'A' && ch <= 'Z') {
        return (ch - 55);
    } else {
        return (ch - 87);
    }
}