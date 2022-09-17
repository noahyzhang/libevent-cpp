// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>

namespace libevent_cpp {

class util_string {
 public:
    static bool is_equals(const std::string& str1, const std::string& str2);
    // 将字符串的中 %xx 进行转换具体的字符
    static std::string string_from_utf8(const std::string& in);

 private:
    // 十六进制的字符转换为整数
    static int hex_to_int(char ch);
};

}  // namespace libevent_cpp
