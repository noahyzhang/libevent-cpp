#ifndef LIBEVENT_CPP_NETWORK_H
#define LIBEVENT_CPP_NETWORK_H

#include <string> 

namespace libevent_cpp {

class util_network {

public:
    static int set_fd_nonblock(int fd);
    static int get_nonblock_socket();
    static int socket_connect(int fd, const std::string& address, unsigned short port);
    static int close_fd(int fd);

private:
    static struct addrinfo* get_addr_info(const std::string& address, unsigned short port);

}; 

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_NETWORK_H