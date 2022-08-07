#include "event_base.h"
#include "util/log/logger.h"

bool libevent_cpp::event_base::add_event(const std::shared_ptr<event>& ev) {
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        // 如果此事件既不可读、又不可写，则不加入
        if (io_ev->is_event_type_removeable()) {
            logger::error("event_base::add_event warn, event with no read or write");
            return false;
        }
        fd_map_io_event_[io_ev->fd_] = io_ev;
        return add(io_ev); 
    } else {
        logger::error("event_base::add_event error, incorrect event type");
        return false;
    }
}

bool libevent_cpp::event_base::remove_event(const std::shared_ptr<event>& ev) {
    if (std::dynamic_pointer_cast<io_event>(ev)) {
        auto io_ev = std::dynamic_pointer_cast<io_event>(ev);
        if (fd_map_io_event_.count(io_ev->fd_) == 0) {
            return true; 
        }
        return remove(io_ev);
    } else {
        logger::error("event_base::remove_event error, incorrect event type");
        return false;
    }
}

void libevent_cpp::event_base::push_event_active_queue(std::shared_ptr<event> ev) {
    active_event_queues_[ev->priority].push(ev);
    ev->set_active();
}