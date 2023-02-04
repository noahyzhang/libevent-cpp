#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "util_network.h"

namespace libevent_cpp {

ssize_t util_network::check_socket_readable_by_select(
    int socket, time_t read_timeout_sec, time_t read_timeout_usec) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    timeval tv{read_timeout_sec, read_timeout_usec};
    return handle_socket_EINTR([&]() {
        select(socket+1, &fds, nullptr, nullptr, &tv);
    });
}

ssize_t util_network::check_socket_writeable_by_select(
    int socket, time_t write_timeout_sec, time_t write_timeout_usec) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    timeval tv{write_timeout_sec, write_timeout_usec};
    return handle_socket_EINTR([&]() {
        select(socket+1, nullptr, &fds, nullptr, &tv);
    });
}

bool util_network::check_socket_alive(int socket) {
    const auto res = check_socket_readable_by_select(socket, 0, 0);
    if (res == 0) {
        // 超时，表明当前没有可读事件。但是可认为连接是存活的
        return true;
    } else if (res < 0 && errno == EBADF) {
        // EBADF  An invalid file descriptor was given in one of the sets.
        // (Perhaps a file descriptor that was already closed, or one on which an error has occurred.)
        return false;
    }
    // select 返回值大于 0，则说明有可读事件
    // 需要再次使用 recv 判断，是错误触发还是真的有读事件需要处理
    char buf[1];
    return recv_socket(socket, &buf[0], sizeof(buf), MSG_PEEK) > 0;
}

ssize_t util_network::recv_socket(int socket, void* buff, size_t size, int flags) {
    return handle_socket_EINTR([&]() {
        return recv(socket, buff, size, flags);
    });
}

ssize_t util_network::send_socket(int socket, const void* buff, size_t size, int flags) {
    return handle_socket_EINTR([&]() {
        return send(socket, buff, size, flags);
    });
}

template <typename T>
ssize_t util_network::handle_socket_EINTR(T fn) {
    ssize_t res = 0;
    for (;;) {
        res = fn();
        // 如果错误是 EINTR，则继续进行操作
        if (res < 0 && errno == EINTR) {
            continue;
        }
        break;
    }
    return res;
}

void util_network::get_remote_ip_and_port(int socket, std::shared_ptr<std::string> ip, std::shared_ptr<uint16_t> port) {
    struct sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    if (!getpeername(socket, reinterpret_cast<struct sockaddr*>(&addr), &addr_len)) {
        // 如果对端是是 unix，则我们获取到对端的 pid、uid、gid 等信息
        if (addr.ss_family == AF_UNIX) {
            struct ucred ucred;
            socklen_t len = sizeof(ucred);
            if (getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &ucred, &len) == 0) {
                *port = ucred.pid;
            }
        }
        get_ip_and_port(addr, addr_len, ip, port);
    }
}

bool util_network::get_ip_and_port(
    const struct sockaddr_storage& addr, socklen_t addr_len,
    std::shared_ptr<std::string> ip, std::shared_ptr<uint16_t> port) {
    
}

}  // namespace libevent_cpp
