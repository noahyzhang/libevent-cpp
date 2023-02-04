// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <map>
#include <string>
#include <memory>
#include <functional>

namespace libevent_cpp {

using Headers = std::multimap<std::string, std::string>;

class http_response;
typedef std::function<void(std::unique_ptr<http_response&&, Headers&&>)> http_client_result_cb;

using Progress = std::function<bool(uint64_t, uint64_t)>;

#define DEFAULT_THREAD_COUNT_POOL 4

// 错误的 socket
#define INVALID_SOCKET (-1)

enum class HttpError {
    Success = 0,
    Unknown,
    Connection,
    BindIPAddress,
    Read,
    Write,
    ExceedRedirectCount,
    Canceled,
    SSLConnection,
    SSLLoadingCerts,
    SSLServerVerification,
    UnsupportedMultipartBoundaryChars,
    Compression,
    ConnectionTimeout,
};

}  // namespace libevent_cpp
