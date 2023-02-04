#include <algorithm>
#include "http_v2/common/util_algorithm.h"

namespace libevent_cpp {

std::string util_algorithm::base64_encode(const std::string& in) {
    static const auto lookup =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(in.size());

    int val = 0;
    int valb = -6;
    for (auto c : in) {
        val = (val << 8) + static_cast<uint8_t>(c);
        valb += 8;
        while (valb >= 0) {
            out.push_back(lookup[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

std::string util_algorithm::encode_url(const std::string& in) {
    std::string result;
    result.reserve(in.size());

    for (size_t i = 0; in[i]; i++) {
        switch (in[i]) {
        case ' ': result += "%20"; break;
        case '+': result += "%2B"; break;
        case '\r': result += "%0D"; break;
        case '\n': result += "%0A"; break;
        case '\'': result += "%27"; break;
        case ',': result += "%2C"; break;
        // case ':': result += "%3A"; break; // ok? probably...
        case ';': result += "%3B"; break;
        default:
            auto c = static_cast<uint8_t>(in[i]);
            if (c >= 0x80) {
                result += '%';
                char hex[4];
                auto len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
                result.append(hex, static_cast<size_t>(len));
            } else {
                result += in[i];
            }
            break;
        }
    }

    return result;
}

}  // namespace libevent_cpp
