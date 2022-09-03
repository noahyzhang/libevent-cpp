#include "connection.h"
#include "request.h"

inline libevent_cpp::http_request* libevent_cpp::http_connection::current_request() {
    if (requests_.empty()) {
        logger::warn("no request"); 
        // TODO 
        close(1);
        return nullptr; 
    }
    return requests_.front().get(); 
}

void libevent_cpp::http_connection::pop_request() {
    if (requests_.empty()) {
        return;
    }
    auto req = std::move(requests_.front());
    requests_.pop();
    if (req->get_cb()) {
        req->get_cb()(req.get());
    }
    req->reset();
    empty_queue_.push(std::move(req));
}

