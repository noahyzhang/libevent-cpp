#include <fcntl.h>
#include <sys/socket.h>
#include "util/util_network.h"
#include "connection.h"


namespace libevent_cpp {

http_connection::http_connection(std::shared_ptr<event_base> event_manager,
    const std::string& host, uint16_t port)
    : event_manager_(event_manager) {
}

int http_connection::create_request(std::unique_ptr<http_request> req) {

}

int http_connection::create_connection(const std::string& server_host, uint16_t server_port) {
    int local_fd = util_network::get_nonblock_socket();
    if (local_fd < 0) {
        return -1;
    }
    // 建立连接
    if (util_network::socket_connect(local_fd, server_host, server_port) < 0) {
        util_network::close_fd(local_fd);
        return -2;
    }
    // 创建 IO 事件
    auto io_ev = std::make_shared<io_event>(local_fd, NONE);
    io_ev->set_event_type(WRITE_ONLY);
    // 添加 IO 事件到事件管理
    event_manager_->register_callback(io_ev, [](std::shared_ptr<io_event> io_ev) {
        if (io_ev->is_read_event_active_status()) {
            // 从 fd 中读信息

        }
        if (io_ev->is_write_event_active_status()) {
            // 向 fd 中写消息
        }
    }, io_ev);
    event_manager_->add_event(io_ev);
}

int http_connection::read_http_response_data(std::shared_ptr<http_response> resp) {
    // 读取 http response 数据
    if (resp->parse_http_response(buffer_) < 0) {
        return -1;
    }
    
}

int http_connection::get_nonblock_socket_fd() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return -1;
    }
    if (set_socket_fd_nonblock(fd) < 0) {
        return -2;
    }
    return fd;
}

int http_connection::set_socket_fd_nonblock(int fd) {
    if (fd < 0) {
        return -1;
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0) {
        return -2;
    }
    if (fcntl(fd, F_SETFD, 1) < 0) {
        return -3;
    }
    return 0;
}

}  // namespace libevent_cpp
