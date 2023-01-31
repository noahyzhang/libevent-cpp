// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <utility>
#include <memory>
#include <string>
#include "util/util_buffer.h"
#include "base/event_base.h"
#include "util/util_logger.h"

namespace libevent_cpp {

class buffer_event : public std::enable_shared_from_this<buffer_event> {
 public:
    buffer_event(std::shared_ptr<event_base> base, int fd);
    ~buffer_event() = default;

    // 注册读事件回调函数
    template <typename F, typename... Rest>
    void register_read_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        read_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    // 注册 eof 事件回调函数
    template <typename F, typename... Rest>
    void register_eof_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        eof_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    // 注册写事件回调函数
    template <typename F, typename... Rest>
    void register_write_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        write_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    // 注册错误事件回调函数
    template <typename F, typename... Rest>
    void register_error_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        error_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    // 设置文件描述符
    inline void set_fd(int fd) { ev_->set_fd(fd); }

    // 获取输入 buffer 的长度
    inline size_t get_input_buf_length() const { return input_->get_cur_length(); }

    // 获取输出 buffer 的长度
    inline size_t get_output_buf_length() const { return output_->get_cur_length(); }

    // 获取输入 buffer 的数据
    inline const char* get_input_buf_data() const { return input_->get_data(); }

    // 获取输出 buffer 的数据
    inline const char* get_output_buf_data() const { return output_->get_data(); }

    // 获取输入的 buffer
    inline std::unique_ptr<buffer>& get_input_buf() { return input_; }

    // 获取输出的 buffer
    inline std::unique_ptr<buffer>& get_output_buf() { return output_; }

    // 获取事件管理对象
    std::shared_ptr<event_base> get_event_base() {
        auto b = base_.lock();
        if (!b) {
            logger::error("buffer_event get_event_base base_ is expired");
        }
        return b;
    }

    // 往 output_ 中写入数据，并将写事件添加到事件管理中
    int write(void* data, size_t size);

    // 从 input_ 中读数据
    int read(void* data, size_t size);

 private:
    // 将事件设置为读事件，并添加到事件管理中
    void add_read_event();

    // 将事件设置为写事件，并添加到事件管理中
    void add_write_event();

    // 清除事件的读状态，并从事件管理中移除
    void remove_read_event();

    // 清除事件的写状态，并从事件管理中移除
    void remove_write_event();

    // 将 output_ 中的数据写入到事件对应的 fd 中
    inline int write_output_buffer() const {
        return output_->write_file(ev_->fd_);
    }

    // 从事件对应的 fd 中读数据到 input_ 内存块中
    inline int read_input_buffer() const {
        return input_->read_file(ev_->fd_, -1);
    }


    // inline int get_fd() const {
    //     return ev_->fd_;
    // }

    // inline size_t write_string(const std::string& str) {
    //     return output_->push_back_string(str);
    // }

    // 获取当前类对象的智能指针
    std::shared_ptr<buffer_event> get_shared_this_ptr() {
        return buffer_event::shared_from_this();
    }

 private:
    // IO 事件的回调
    static void io_callback(std::shared_ptr<buffer_event> bev);

 protected:
    // 输入的内存块
    std::unique_ptr<buffer> input_;
    // 输出的内存块
    std::unique_ptr<buffer> output_;
    // IO 事件
    std::shared_ptr<io_event> ev_;
    // 事件管理
    std::weak_ptr<event_base> base_;

 public:
    // 读事件回调函数
    std::shared_ptr<Callback> read_cb_ = nullptr;
    // eof 事件回调函数
    std::shared_ptr<Callback> eof_cb_ = nullptr;
    // 写事件回调函数
    std::shared_ptr<Callback> write_cb_ = nullptr;
    // 错误事件回调函数
    std::shared_ptr<Callback> error_cb_ = nullptr;
};

}  // namespace libevent_cpp
