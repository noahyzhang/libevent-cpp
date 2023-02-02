// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <regex>
#include <utility>
#include <memory>
#include "base/event_base.h"
#include "request.h"
#include "response.h"
#include "util/thread_pool.h"
#include "common/thread_task_handler.h"

namespace libevent_cpp {

// http 服务端
class http_server {
public:
    using Handler = std::function<void(const http_request&, http_response&)>;
    // using HandlerWithContentReader = std::function<void(const Request&, Response&, const ContentReader&)>;

public:
    // 设置线程池中线程的个数
    void set_thread_count_pool(size_t thread_num);

private:
    // http 服务端监听事件处理的回调函数
    static void listen_event_cb();

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

private:
    // 线程池
    std::shared_ptr<thread_pool> thread_pool_ = nullptr;
    // 事件管理
    std::shared_ptr<event_base> event_manager_ = nullptr;
    // 所有的线程处理类
    std::vector<std::shared_ptr<thread_task_handler>> thread_task_handlers_;
};

}  // namespace libevent_cpp
