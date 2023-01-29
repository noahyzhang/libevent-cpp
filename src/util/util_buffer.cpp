// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <string.h>
#include <unistd.h>
#include <string>
#include "util/util_buffer.h"
#include "util/util_logger.h"

void libevent_cpp::buffer::reset() {
    remove_front(cur_len_);
}

void libevent_cpp::buffer::resize(size_t n) {
    remove_front(cur_len_);
    expand_buffer(n);
}

std::string libevent_cpp::buffer::readline() {
    char* data = (char*)buf_;
    size_t index;
    for (index = 0; index < cur_len_; index++) {
        if (data[index] == '\r' || data[index] == '\n') {
            break;
        }
    }
    // 没有找到换行符
    if (index == cur_len_) return "";
    // 检测 '\r\n' 或 '\n\r' 的情况
    int is_next_index = 0;
    if (index + 1 < cur_len_) {
        if ((data[index] == '\r' && data[index+1] == '\n') ||
            (data[index] == '\n' && data[index+1] == '\r')) {
            is_next_index = 1;
        }
    }
    data[index] = '\0';
    data[index+is_next_index] = '\0';
    std::string line(data);

    return line;
}

int libevent_cpp::buffer::push_back(void* data, size_t data_len) {
    // 检测是否需要扩展
    // TODO 
    size_t need_space = cur_len_ + data_len;
    if (total_len_ < need_space) {
        if (expand_buffer(data_len) == -1) {
            return -1;
        }
    }
    // 尾插
    memcpy(buf_ + cur_len_, data, data_len);
    cur_len_ += data_len;
    return 0;
}

// int libevent_cpp::buffer::push_back_buffer(std::unique_ptr<buffer>& buf, size_t buf_len) {
//     if (!buf) return 0;
//     int real_len = buf_len;
//     if (real_len > buf->cur_len_ || buf_len < 0) {
//         real_len = buf->cur_len_;
//     }
//     int res = push_back(buf->buf_, real_len);
//     if (res == 0) {
//         // TODO 
//     }
//     return res;
// }

size_t libevent_cpp::buffer::pop_front(void* data, size_t data_len) {
    if (cur_len_ < data_len) {
        data_len = cur_len_;
    }
    memcpy(data, buf_, data_len);
    remove_front(data_len);
    return data_len;
}

int libevent_cpp::buffer::read_file(size_t fd, int size) {
    if (size < 0 || size > DEFAULT_ONCE_READ_FILE_MAX_BYTES) {
        size = DEFAULT_ONCE_READ_FILE_MAX_BYTES;
    }

    if (expand_buffer(size) == -1) {
        return -1;
    }
    unsigned char* ptr = buf_ + cur_len_;
    int real_read_size = read(fd, ptr, size);
    if (real_read_size <= 0) {
        return real_read_size;
    }
    cur_len_ += real_read_size;
    return real_read_size;
}

size_t libevent_cpp::buffer::write_file(size_t fd) {
    int real_write_size = write(fd, buf_, cur_len_);
    if (real_write_size < 0) {
        return real_write_size;
    }
    remove_front(real_write_size);
    return real_write_size;
}

int libevent_cpp::buffer::expand_buffer(size_t data_len) {
    size_t need_space = start_index_ + cur_len_ + data_len;
    // 当前空间足够，无需扩展空间
    if (total_len_ >= need_space) return 0;
    // 被标记删除的空间足够当前需要扩展的长度
    if (start_index_ >= data_len) {
        reset_buffer();
    } else {
        unsigned char* new_buf;
        size_t new_len = total_len_;
        if (new_len < 256) new_len = 256;
        while (new_len < need_space) {
            new_len <<= 1;
        }
        if (origin_buf_ != buf_) {
            reset_buffer();
        }
        if ((new_buf = (unsigned char*)realloc(buf_, new_len)) == nullptr) {
            logger::error("buffer expand_buffer realloc error");
            return -1;
        }
        origin_buf_ = buf_ = new_buf;
        total_len_ = new_len;
    }
    return 0;
}

void libevent_cpp::buffer::remove_front(size_t len) {
    if (len >= cur_len_) {
        cur_len_ = 0;
        buf_ = origin_buf_;
        start_index_ = 0;
    } else {
        buf_ += len;
        start_index_ += len;
        cur_len_ -= len;
    }
}

void libevent_cpp::buffer::reset_buffer() {
    memmove(origin_buf_, buf_, cur_len_);
    buf_ = origin_buf_;
    start_index_ = 0;
}

unsigned char* libevent_cpp::buffer::find(unsigned char* what, size_t len) {
    size_t remain = cur_len_;
    auto search = buf_;
    unsigned char* p;
    while ((p = (unsigned char*)memchr(search, *what, remain)) != nullptr && remain > len) {
        if (memcmp(p, what, len) == 0) {
            return (unsigned char*)p;
        }
        search = p + 1;
        remain = cur_len_ - (search - buf_);
    }
    return nullptr;
}
