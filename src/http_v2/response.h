// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include "common.h"

namespace libevent_cpp {

class Response {
public:
    // 设置头部
    void set_header(const std::string& key, const std::string& value);
    // 设置正文
    void set_content(const std::string& content);

private:
    // http 的版本
    std::string version_;
    // 状态码
    int status_code_;
    // 状态码的文本秒数
    std::string status_reason_;
    // 头部
    Headers headers_;
};

}  // namespace libevent_cpp
