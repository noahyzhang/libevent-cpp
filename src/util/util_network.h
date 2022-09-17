#ifndef LIBEVENT_CPP_NETWORK_H
#define LIBEVENT_CPP_NETWORK_H

#include <string>
#include <memory>

namespace libevent_cpp {

class util_network {

public:
    static int set_fd_nonblock(int fd);
    static int get_nonblock_socket();
    static int socket_connect(int fd, const std::string& address, unsigned short port);
    static int close_fd(int fd);
    static int bind_socket(const std::string& address, unsigned short port, bool reuse = true);
    static int listen_fd(int fd);
    static int accept_socket(int fd, std::shared_ptr<td::string> host, std::shared_ptr<int> port);

private:
    static struct addrinfo* get_addr_info(const std::string& address, unsigned short port);

}; 

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_NETWORK_H