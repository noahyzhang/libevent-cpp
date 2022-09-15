#ifndef LIBEVENT_CPP_SERVER_H
#define LIBEVENT_CPP_SERVER_H

#include <memory>
#include <map>
#include <string>
#include "thread/thread_pool.h"
#include "base/event_base.h"
#include "common/request.h"

namespace libevent_cpp {

using HandleCallBack = std::function<void(http_request*)>;

class http_server {

public: 
    http_server();
    ~http_server();
    void resize_thread_pool(size_t thread_num);
    inline size_t get_idle_thread_num() {
        return pool_->get_idle_thread_num();
    }
    inline void set_timeout(int sec) {
        timeout_ = sec;
    }
    inline void set_handle_cb(std::string what, HandleCallBack cb) {
        handle_callbacks_[what] = cb;
    }
    int run(const std::string& address, unsigned short port);

private:
    std::shared_ptr<thread_pool> pool_ = nullptr;
    std::shared_ptr<event_base> base_ = nullptr;
    std::map<std::string, HandleCallBack> handle_callbacks_;
    int timeout_ = -1;

};

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_SERVER_H