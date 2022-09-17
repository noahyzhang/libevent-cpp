#include "http/server/server.h"
#include "base/epoll_base.h"
#include "util/util_network.h"

libevent_cpp::http_server::http_server() {
    base_ = std::make_shared<libevent_cpp::epoll_base>();
    pool_ = std::make_shared<libevent_cpp::thread_pool>();
}

libevent_cpp::http_server::~http_server() {
    for (size_t i = 0; i < threads_.size(); i++) {
        // TODO 
    }
}

void libevent_cpp::http_server::dispatch_task(http_server_thread* thread) {
    thread->dispatch();
}

void libevent_cpp::http_server::resize_thread_pool(size_t thread_num) {
    pool_->reset_thread_num(thread_num);
    int cur_threads = threads_.size();
    if (cur_threads <= thread_num) {
        for (int i = cur_threads; i < thread_num; i++) {
            threads_.emplace_back(std::unique_ptr<http_server_thread>(new http_server_thread(this)));
        }
    } else {
        threads_.resize(thread_num);
    }

    for (int i = 0; i < thread_num; i++) {
        pool_->push(dispatch_task, threads_[i].get());
    }
}

void libevent_cpp::http_server::listen_cb(int fd, http_server* server) {
    std::string host;
    int port;
    int peer_fd = util_network::accept_socket(fd, std::make_shared<std::string>(host), std::make_shared<int>(port));

    logger::info("http server accept new client with fd: %d, host: %s, port: %d", peer_fd, host.c_str(), port);

    server->client_info_queue_.push(std::unique_ptr<http_client_info>(new http_client_info{peer_fd, host, port}));
    // TODO 
}

int libevent_cpp::http_server::run(const std::string& address, unsigned short port) {
    if (threads_.size() == 0) {
        resize_thread_pool(4);
    }
    int fd = util_network::bind_socket(address, port, true);
    if (fd < 0) {
        exit(-1);
    }
    if (util_network::listen_fd(fd) < 0) {
        return -1;
    }
    // 使用一个读事件去 accept 文件描述符
    auto ev = create_event<io_event>(fd, READ_ONLY);
    base_->register_callback(ev, listen_cb, fd, this);
    // TODO 
    base_->add_event(ev);

    logger::info("http server listening on fd: %d, bound to port: %d, awaiting connections.");

    base_->start_dispatch();
    return 0;
}