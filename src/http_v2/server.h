// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <regex>
#include <utility>
#include "request.h"
#include "response.h"

namespace libevent_cpp {

// http 服务端
class http_server {
public:
    using Handler = std::function<void(const Request&, Response&)>;
    // using HandlerWithContentReader = std::function<void(const Request&, Response&, const ContentReader&)>;

public:
    void Get(const std::string& pattern, Handler handler);
    void Post(const std::string& pattern, Handler handler);
    void Put(const std::string& pattern, Handler handler);
    void Patch(const std::string& pattern, Handler handler);
    void Delete(const std::string& pattern, Handler handler);
    void Options(const std::string& pattern, Handler handler);
    int listen_and_serve(const std::string& host, uint16_t port);
    int listen_and_serve_TLS();

private:
    using Handlers = std::vector<std::pair<std::regex, Handler>>;
    // http get 方法的请求处理集合
    Handlers get_handlers_;
    // http post 方法的请求处理集合
    Handlers post_handlers_;
    // http put 方法的请求处理集合
    Handlers put_headers_;
    // http patch 方法的请求处理集合
    Handlers patch_headers_;
    // http delete 方法的请求处理集合
    Handlers delete_headers_;
    // http options 方法的请求处理集合
    Handlers options_handlers_;
};

}  // namespace libevent_cpp
