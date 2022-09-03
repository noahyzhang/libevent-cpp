#ifndef LIBEVENT_CPP_HTTP_CONNECTION_H
#define LIBEVENT_CPP_HTTP_CONNECTION_H

#include "event/time_event.h"
#include "buffer_event/buffer_event.h"
#include "util/util_logger.h"

namespace libevent_cpp {

enum http_connection_error {
    HTTP_TIMEOUT,
    HTTP_EOF,
    HTTP_INVALID_HEADER
};

enum http_connection_state {
    DISCONNECTED,  // 未连接
    CONNECTING, // 连接尝试中
    IDLE,  // 连接已建立
    READING_FIRSTLINE, // 读取请求行/状态行
    READING_HEADERS, // 读取请求/返回的报头
    READING_BODY, // 读取请求/返回的数据部分
    READING_TRAILER, // 
    WRITING, // 写入请求/返回报头或数据
    CLOSED // 连接关闭
};

class http_connection : public buffer_event {

public:
    http_connection(std::shared_ptr<event_base> base, int fd); 
    virtual ~http_connection(); 
    virtual void do_read_done() = 0;
    virtual void do_write_done() = 0;

    inline int is_connected() const {
        switch (state_) {
        case DISCONNECTED:
        case CONNECTING:
            return 0;
        case IDLE:
        case READING_FIRSTLINE:
        case READING_HEADERS:
        case READING_BODY:
        case READING_TRAILER:
        case WRITING:
        default:
            return 1;
        }
    }

    inline bool is_closed() {
        return CLOSED == state_;
    }

protected:
    inline http_request* current_request();
    inline void pop_request(); 

protected:
    int timeout_ = -1; 
    enum http_connection_state state_; 
    std::queue<std::unique_ptr<http_request>> requests_;
    std::queue<std::unique_ptr<http_request>> empty_queue_; 
    std::shared_ptr<time_event> read_timer_ = nullptr;
    std::shared_ptr<time_event> write_timer_ = nullptr;
};

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_HTTP_CONNECTION_H 