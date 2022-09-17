// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <utility>
#include <memory>
#include "util/util_buffer.h"
#include "base/event_base.h"
#include "util/util_logger.h"

namespace libevent_cpp {

class buffer_event {
 public:
    buffer_event(std::shared_ptr<event_base> base, int fd);
    ~buffer_event() = default;

    template <typename F, typename... Rest>
    void register_read_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        read_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    template <typename F, typename... Rest>
    void register_eof_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        eof_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    template <typename F, typename... Rest>
    void register_write_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        write_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    template <typename F, typename... Rest>
    void register_error_cb(F&& f, Rest&&... rest) {
        auto task = std::bind(std::forward<F>(f), std::forward<Rest>(rest)...);
        error_cb_ = std::make_shared<Callback>([task]() { task(); });
    }

    inline void set_fd(int fd) { ev_->set_fd(fd); }

    inline size_t get_input_buf_length() const { return input_->get_cur_length(); }
    inline size_t get_output_buf_length() const { return output_->get_cur_length(); }
    inline const char* get_input_buf_data() const { return input_->get_data(); }
    inline const char* get_output_buf_data() const { return output_->get_data(); }

    std::shared_ptr<event_base> get_event_base() {
        auto b = base_.lock();
        if (!b) {
            logger::error("buffer_event get_event_base base_ is expired");
        }
        return b;
    }

    int write(void* data, size_t size);
    int read(void* data, size_t size);
    void add_read_event();
    void add_write_event();
    void remove_read_event();
    void remove_write_event();

    inline int write_output_buffer() const {
        return output_->write_file(ev_->fd_);
    }
    inline int read_input_buffer() const {
        return input_->read_file(ev_->fd_, -1);
    }
    inline int get_fd() const {
        return ev_->fd_;
    }

 private:
    static void io_callback(buffer_event* bev);

 protected:
    std::unique_ptr<buffer> input_;
    std::unique_ptr<buffer> output_;
    std::shared_ptr<io_event> ev_ = nullptr;
    std::weak_ptr<event_base> base_;

 public:
    std::shared_ptr<Callback> read_cb_ = nullptr;
    std::shared_ptr<Callback> eof_cb_ = nullptr;
    std::shared_ptr<Callback> write_cb_ = nullptr;
    std::shared_ptr<Callback> error_cb_ = nullptr;
};

}  // namespace libevent_cpp
