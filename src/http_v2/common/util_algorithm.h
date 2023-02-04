// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>

namespace libevent_cpp {

class util_algorithm {
public:
    // 对输入进行 base64 编码
    static std::string base64_encode(const std::string& in);
    // 对 url 进行编码
    static std::string encode_url(const std::string& in);
};

}  // namespace libevent_cpp
