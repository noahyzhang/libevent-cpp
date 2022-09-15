#include "http/server/server.h"

void home_cb(libevent_cpp::http_request* req) {
    auto buf = std::unique_ptr<libevent_cpp::buffer>(new libevent_cpp::buffer());
    buf->push_back_string("hello world");
    // TODO
    req->send_reply(HTTP_OK, "everything is fine", std::move(buf));
}

int main() {
    auto http_server = std::unique_ptr<libevent_cpp::http_server>(new libevent_cpp::http_server());
    http_server->set_timeout(15);
    http_server->set_handle_cb("/", home_cb);
    http_server->resize_thread_pool(4);
    http_server->run("localhost", 8888);
    return 0;
}