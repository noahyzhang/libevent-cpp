#include <stdlib.h>
#include "select_base.h"
#include "util/log/logger.h"

#define MAX_SELECT_FD_SIZE 1024 

bool libevent_cpp::select_base::init() {
    // int fdsz = ((32 + NFDBITS) / sizeof(NFDBITS)) * sizeof(fd_mask);

    fd_set* new_set = nullptr;
    new_set = (fd_set*)realloc(event_read_set_in, fdsz);
    event_read_set_in = new_set;

    new_set = (fd_set*)realloc(event_write_set_in, fdsz);
    event_write_set_in = new_set;

    new_set = (fd_set*)realloc(event_read_set_out, fdsz);
    event_read_set_out = new_set;

    new_set = (fd_set*)realloc(event_write_set_out, fdsz);
    event_write_set_out = new_set;
}

libevent_cpp::select_base::~select_base() {
    free(event_read_set_in);
    event_read_set_in = nullptr;
    free(event_write_set_in);
    event_write_set_in = nullptr;
    free(event_read_set_out);
    event_read_set_out = nullptr;
    free(event_write_set_out);
    event_write_set_out = nullptr;
}

bool libevent_cpp::select_base::add(std::shared_ptr<io_event> ev) {
    if (ev->fd_ > MAX_SELECT_FD_SIZE) {
        logger::error("select_base add select fd > MAX_SELECT_FD_SIZE: %d", MAX_SELECT_FD_SIZE);
        return false;
    }

}

bool libevent_cpp::select_base::remove(std::shared_ptr<io_event> ev) {
    if (!ev->is_event_type_readable()) {
        FD_CLR(ev->fd_, event_read_set_in);
    }
    if (!ev->is_event_type_writeable()) {
        FD_CLR(ev->fd_, event_write_set_in);
    }
    return true; 
}

bool libevent_cpp::select_base::dispatch(struct timeval* tv) {
    
}