// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <list>
#include <queue>
#include <memory>
#include <map>
#include <string>
#include <sys/eventfd.h>
#include "event/signal_event.h"
#include "http/server/server_connection.h"

namespace libevent_cpp {

// 线程执行类
class http_server_thread : public std::enable_shared_from_this<http_server_thread> {
 public:
    http_server_thread(
       std::shared_ptr<concurrent_queue<http_client_info>> client_info_queue,
       std::shared_ptr<std::map<std::string, HandleCallBack>> handle_callbacks);
    ~http_server_thread();

    // 此线程开始监听
    void dispatch();

    // 设置此线程结束
    void set_terminate();

    // 唤醒此线程
    void wakeup();

 private:
    // 获取当前类调用对象的智能指针
    std::shared_ptr<http_server_thread> get_shared_this_ptr();

    // 
    std::unique_ptr<http_server_connection> get_empty_connection();

 private:
    // 获取一个连接
    static void get_connections(std::shared_ptr<io_event> ev, std::shared_ptr<http_server_thread> thread);
    static void ev_sigpipe_handler(std::shared_ptr<signal_event> ev);
    // 读取被唤醒的事件对应 fd 的信息
    static int read_wake_msg(int fd);

 private:
    // 存储客户端信息的队列
    std::shared_ptr<concurrent_queue<http_client_info>> client_info_queue_ = nullptr;
    // 存储服务器不同路径对应的回调函数
    std::shared_ptr<std::map<std::string, HandleCallBack>> handle_callbacks_ = nullptr;
    // 事件管理
    std::shared_ptr<event_base> base_ = nullptr;
    // IO 事件，用于唤醒唤醒（使用内核提供的异步通知事件）
    std::shared_ptr<io_event> io_ev_ = nullptr;
    // 信号事件（处理SIGPIPE 信号）
    std::shared_ptr<signal_event> pipe_sig_ev_ = nullptr;
    std::list<std::unique_ptr<http_server_connection>> connection_list_;
    std::queue<std::unique_ptr<http_server_connection>> empty_queue_;
};

}  // namespace libevent_cpp

