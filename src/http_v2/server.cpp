#include "server.h"

namespace libevent_cpp {

void http_server::Get(const std::string& pattern, Handler handler) {
    get_handlers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Post(const std::string& pattern, Handler handler) {
    post_handlers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Put(const std::string& pattern, Handler handler) {
    put_headers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Patch(const std::string& pattern, Handler handler) {
    patch_headers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Delete(const std::string& pattern, Handler handler) {
    delete_headers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

void http_server::Options(const std::string& pattern, Handler handler) {
    options_handlers_.emplace_back(std::make_pair(std::regex(pattern), std::move(handler)));
}

int http_server::listen_and_serve(const std::string& host, uint16_t port) {
    
}

}  // namespace libevent_cpp
