#include <algorithm>
#include "http_v2/common/util.h"

namespace libevent_cpp {

bool http_util::is_hex_digit_and_convert_dec(char ch, int& res) {
    if (std::isdigit(ch)) {
        res = ch - '0';
        return true;
    } else if (ch >= 'A' && ch <= 'F') {
        res = ch - 'A' + 10;
        return true;
    } else if (ch >= 'a' && ch <= 'f') {
        res = ch - 'a' + 10;
        return true;
    }
    return false;
}

std::string http_util::decode_url(const std::string& url, bool convert_plus_to_space) {
    std::string res;
    for (size_t i = 0; i < url.size(); ++i) {
        if (url[i] == '%' && i + 1 < url.size()) {
            if (url[i+1] == 'u') {
                int val = 0;

            }
        }
    }
}

}  // namespace libevent_cpp
