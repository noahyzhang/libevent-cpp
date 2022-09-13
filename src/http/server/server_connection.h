#ifndef LIBEVENT_CPP_SERVER_CONNECTION_H
#define LIBEVENT_CPP_SERVER_CONNECTION_H 

#include "http/common/connection.h"
#include "http/server/server.h"

namespace libevent_cpp {

class http_server_connection : public http_connection {

public:
    http_server_connection(std::shared_ptr<event_base> base, int fd, http_server* server);
    ~http_server_connection() = default;

    int create_request();

    void fail(http_connection_error err) override;
    void do_read_done() override;
    void do_write_done() override;

private:
    void handle_request(http_request* req);

public:
    std::string get_client_address() const {
        return client_address_;
    }
    unsigned int get_client_port() const {
        return client_port_;
    }

private:
    http_server* server_;
    std::string client_address_;
    unsigned int client_port_;
};

static void read_timeout_cb(http_server_connection* conn);
static void write_timeout_cb(http_server_connection* conn);

} // namespace libevent_cpp 

#endif 