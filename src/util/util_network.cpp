#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "util_network.h"
#include "util/log/logger.h"


int libevent_cpp::util_network::set_fd_nonblock(int fd) {
    if (fd < 0) {
        logger::error("set_fd_nonblock failed, fd: %d < 0", fd);
        return -1; 
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        logger::error("set_fd_nonblock failed, fcntl set nonblock err fd: %d", fd); 
        return -1;
    }
    if (fcntl(fd, F_SETFD, 1) == -1) {
        logger::error("set_fd_nonblock failed, fcntl set fd err fd: %d", fd); 
        return -1; 
    }
    return 0;
}

int libevent_cpp::util_network::get_nonblock_socket() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        logger::error("get_nonblock_socket failed, socket err"); 
        return -1; 
    }
    if (set_fd_nonblock(fd) < 0) {
        return -1;
    }
    return fd; 
}

int libevent_cpp::util_network::socket_connect(
    int fd, const std::string& address, unsigned short port) {
    struct addrinfo* ai = get_addr_info(address, port);
    if (!ai) {
        logger::error("socket_connect failed, addr info is nullptr");
        return -1;
    }
    if (connect(fd, ai->ai_addr, ai->ai_addrlen) < 0) {
        if (errno != EINPROGRESS) {
            logger::error("socket_connect failed, errno: %d", errno);
            freeaddrinfo(ai);
            return -1;
        }
    }
    logger::info("socket_connect success connect to address: %s, port: %d", address, port);
    freeaddrinfo(ai);
    return 0;
}

int libevent_cpp::util_network::close_fd(int fd) {
    if (fd <= 0) {
        logger::warn("close fd: %d err", fd);
        return -1;
    }
    logger::info("close fd: %d success", fd);
    return close(fd);
}

struct addrinfo* libevent_cpp::util_network::get_addr_info(
    const std::string& address, unsigned short port) {
    std::string port_str = std::to_string(port);
    struct addrinfo* aitop, hints;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // 如果 nodename 为空，则返回的地址是 INADDR_ANY
    // 如果 nodename 不为空，则此标志会忽略
    hints.ai_flags = AI_PASSIVE; 
    int res = getaddrinfo(address.c_str(), port_str.c_str(), &hints, &aitop);
    if (res < 0) {
        if (res == EAI_SYSTEM) {
            logger::error("get_addr_info failed, EAI_SYSTEM");
        } else {
            logger::error("get_addr_info err: %s", gai_strerror(res)); 
        }
        return nullptr; 
    }
    return aitop; 
}