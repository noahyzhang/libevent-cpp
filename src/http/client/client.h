#ifndef LIBEVENT_CPP_CLIENT_H
#define LIBEVENT_CPP_CLINET_H

#include <memory> 
#include "base/event_base.h"
#include "client_connection.h" 

namespace libevent_cpp {

// 当类被share_ptr管理，且在类的成员函数里需要把当前类对象作为参数传给其他函数时，就需要传递一个指向自身的share_ptr
class http_client {

public:
    int timeout_ = -1;
    std::shared_ptr<event_base> base_ = nullptr;

public:
    http_client();
    ~http_client() = default; 

    inline void set_timeout(int sec) {
        timeout_ = sec;
    }
    std::unique_ptr<http_client_connection> make_connection(const std::string& server_address, unsigned int server_port);
    void run(); 

}; 

} // namespace libevent_cpp 

#endif // LIBEVENT_CPP_CLIENT_H 