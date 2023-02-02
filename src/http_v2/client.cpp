#include "base/epoll_base.h"
#include "client.h"

namespace libevent_cpp {

http_client::http_client(const std::string& host, uint16_t port)
    : server_host_(host), server_port_(port) {
    // 为了实现异步接口
    event_manager_ = std::make_shared<epoll_base>();
    // http 连接
    http_connection_ = std::make_shared<http_connection>(event_manager_, host, port);
}

void http_client::GetAsync(const std::string& path, http_client_result_cb cb) {
    std::unique_ptr<http_request> req(new http_request());
    req->set_request_method("GET");
    req->set_request_path(path);
    req->set_client_result_cb(cb);
    http_connection_->create_request(std::move(req));
    event_manager_dispatch();
}

void http_client::event_manager_dispatch() {
    // 局部静态变量只会初始化一次
    static auto once_call = [&]() {
        if (event_manager_) {
            event_manager_->start_dispatch();
        }
    }();
}

}  // namespace libevent_cpp
