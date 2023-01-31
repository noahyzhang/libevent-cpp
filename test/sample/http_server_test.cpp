// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <memory>
#include "http/server/server.h"

void home_page(libevent_cpp::http_request* req) {
    auto buf = std::unique_ptr<libevent_cpp::buffer>(new libevent_cpp::buffer());
    buf->push_back_string("hello, This is home page");

    req->send_reply(HTTP_OK, "Everything is fine.", req->input_headers_["Empty"].empty() ? std::move(buf) : nullptr);
}

int main() {
    auto http_server = std::unique_ptr<libevent_cpp::http_server>(new libevent_cpp::http_server());
    http_server->set_timeout(5);

    http_server->set_handle_cb("/", home_page);
    http_server->resize_thread_pool(4);
    http_server->run("127.0.0.1", 8888);

    return 0;
}
