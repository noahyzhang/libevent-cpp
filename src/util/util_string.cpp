// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <sstream>
#include <regex>
#include <utility>
#include <vector>
#include "util/util_string.h"

namespace libevent_cpp {

bool util_string::is_equals(
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

std::string util_string::string_from_utf8(const std::string& in) {
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

int util_string::hex_to_int(char ch) {
    if (ch >= '0' && ch <= '9') {
        return (ch - 48);
    } else if (ch >= 'A' && ch <= 'Z') {
        return (ch - 55);
    } else {
        return (ch - 87);
    }
}

std::string util_string::convert_str_to_html(const std::string& str) {
    static std::vector<std::pair<std::string, std::string>> cond{
        {"<", "&lt"}, {">", "&gt"}, {"\"", "&quot"}, {"'", "&#039"}, {"&", "&amp"}};
    std::string res(str);
    auto replace_func = [](const std::string& str, const std::string& from,
        const std::string& to) -> std::string {
        if (from.empty()) return str;
        std::regex re("\\" + from);
        return std::regex_replace(str, re, to);
    };
    std::for_each(cond.begin(), cond.end(), [&](const std::pair<std::string, std::string>& content){
        res = replace_func(res, content.first, content.second);
    });
    return res;
}

std::vector<std::string> util_string::split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream iss(str);
    while (std::getline(iss, token, delimiter)) {
        tokens.emplace_back(token);
    }
    return tokens;
}

bool util_string::has_crlf(const std::string& str) {
    auto p = str.c_str();
    while (*p) {
        if (*p == '\r' || *p == '\n') return true;
        p++;
    }
    return false;
}

}  // namespace libevent_cpp
