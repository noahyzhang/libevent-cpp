// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include <vector>

namespace libevent_cpp {

class util_string {
 public:
    static bool is_equals(const std::string& str1, const std::string& str2);

    // 将字符串的中 %xx 进行转换具体的字符
    static std::string string_from_utf8(const std::string& in);

    // 转换字符串中的某些字符使其满足 html 格式
    static std::string convert_str_to_html(const std::string& str);

    // 分割字符串
    static std::vector<std::string> split_string(const std::string& str, char delimiter);

    // 判断字符串中是否有回车/换行符
    static bool has_crlf(const std::string& str);

 private:
    // 十六进制的字符转换为整数
    static int hex_to_int(char ch);
};

}  // namespace libevent_cpp
