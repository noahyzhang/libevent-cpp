// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>

namespace libevent_cpp {

class http_util {
public:
    // 解码 url
    std::string decode_url(const std::string& url, bool convert_plus_to_space);
    // 判断一个字符是否为十六进制，如果是转换为十进制
    bool is_hex_digit_and_convert_dec(char ch, int& res);

};

}  // namespace libevent_cpp
