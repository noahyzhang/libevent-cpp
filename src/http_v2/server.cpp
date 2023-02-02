#include "common/common.h"
#include "util/util_network.h"
#include "util/util_logger.h"
#include "server.h"

namespace libevent_cpp {

void http_server::Get(const std::string& pattern, Handler handler) {
    get_handlers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Post(const std::string& pattern, Handler handler) {
    post_handlers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Put(const std::string& pattern, Handler handler) {
    put_headers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Patch(const std::string& pattern, Handler handler) {
    patch_headers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Delete(const std::string& pattern, Handler handler) {
    delete_headers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Options(const std::string& pattern, Handler handler) {
    options_handlers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

int http_server::listen_and_serve(const std::string& host, uint16_t port) {
    if (thread_task_handlers_.size() <= 0) {
        set_thread_count_pool(DEFAULT_THREAD_COUNT_POOL);
    }
    int fd = util_network::bind_socket(host, port, true);
    if (fd < 0) {
        logger::error("bind socket failed, host: %s, port: %d", host, port);
        return -1;
    }
    if (util_network::listen_fd(fd) < 0) {
        logger::error("listen fd failed, fd: %d", fd);
        return -2;
    }
    // 使用一个读事件去 accept 客户端
    auto io_ev = create_event<io_event>(fd, READ_ONLY);
    event_manager_->register_callback(io_ev, [](int fd){
        auto host = std::make_shared<std::string>();
        auto port = std::make_shared<int>();
        int peer_fd = util_network::accept_socket(fd, host, port);
        logger::debug("http server accept new client with fd: %d, host: %d, port: %d", peer_fd, host, port);
        // 处理此客户端请求
        
    }, fd);
    event_manager_->add_event(io_ev);

    logger::debug("http server listening on fd: %d, bound to port: %d, awaiting connections.", fd, port);
    event_manager_->start_dispatch();
    return 0;
}

void http_server::set_thread_count_pool(size_t thread_num) {
    // 想要设置的线程数量和当前线程池中的数量相等，不用做操作
    int cur_thread_num = thread_task_handlers_.size();
    if (cur_thread_num == thread_num) return;
    // 设置线程池的数量
    thread_pool_->reset_thread_num(thread_num);
    // 扩容
    if (cur_thread_num < thread_num) {
        for (size_t i = cur_thread_num; i < thread_num; ++i) {
            // 保存此线程任务
            thread_task_handlers_.emplace_back(std::make_shared<thread_task_handler>());
            // 向线程池中添加此线程任务，通过回调由线程来执行想要任务
            thread_pool_->push([](std::shared_ptr<thread_task_handler> handler) {
                handler->dispatch();
            }, thread_task_handlers_[i]->get_shared_this_ptr());
        }
    } else {
        // 缩容
        thread_task_handlers_.resize(thread_num);
    }
}

}  // namespace libevent_cpp
