// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <map>
#include <string>
#include <memory>

namespace libevent_cpp {

using Headers = std::multimap<std::string, std::string>;

class http_response;
typedef std::function<void(std::unique_ptr<http_response&&, Headers&&>)> http_client_result_cb;

#define DEFAULT_THREAD_COUNT_POOL 4

// 错误的 socket
#define INVALID_SOCKET (-1)

}  // namespace libevent_cpp
