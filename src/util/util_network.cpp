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

int libevent_cpp::util_network::bind_socket(const std::string& address, unsigned short port, bool reuse = true) {
    struct addrinfo* addr_info = nullptr;
    if (address.empty() || port == 0) {
        return -1;
    }
    if (!get_addr_info(address, port)) {
        return -1;
    }
    int fd = get_nonblock_socket();
    if (fd < 0) {
        goto exit;
    }
    int flag_switch = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<void*>(&flag_switch), sizeof(flag_switch));
    if (reuse) {
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<void*>(&flag_switch), sizeof(flag_switch));
    }
    if (addr_info) {
        if (bind(fd, addr_info->ai_addr, addr_info->ai_addrlen) < 0) {
            logger::error("bind socket error");
            goto exit;
        }
    }
    freeaddrinfo(addr_info);
    return fd;
exit:
    freeaddrinfo(addr_info);
    close(fd);
    return -1;
}

int libevent_cpp::util_network::listen_fd(int fd) {
    if (listen(fd, 128) < 0) {
        logger::error("listen error");
        close(fd);
        return -1;
    }
    return 0;
}

int libevent_cpp::util_network::accept_socket(int fd, std::shared_ptr<std::string> host, std::shared_ptr<int> port) {
    struct sockaddr_storage ss_client;  // 通用的地址数据结构
    struct sockaddr* sa = (struct sockaddr_storage*)&ss_client;
    socklen_t client_addr_len = sizeof(ss_client);

    int sock_fd = accept(fd, (struct sockaddr*)&ss_client, &client_addr_len);
    if (sock_fd < 0) {
        if (errno != EAGAIN && errno != EINTR) {
            logger::error("accept failed, errno: %d", errno);
        }
        return -1;
    }
    if (set_fd_nonblock(sock_fd) < 0) {
        return -1;
    }
    char tmp_host[NI_MAXHOST];
    char tmp_serv[NI_MAXSERV];
    int res = getnameinfo(sa, client_addr_len, tmp_host, sizeof(tmp_host),
            tmp_serv, sizeof(tmp_serv), NI_NUMERICHOST | NI_NUMERICSERV);
    if (res != 0) {
        logger::error("getnameinfo err: %s", gai_strerror(res));
        return -1;
    }
    host = std::string(tmp_host);
    port = std::stoi(tmp_serv);
    return sock_fd;
}

