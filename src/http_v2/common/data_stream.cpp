#include <string.h>
#include <algorithm>
#include <limits>
#include <thread>
#include <chrono>
#include "common/util_network.h"
#include "common/data_stream.h"

namespace libevent_cpp {

template <typename... Args>
ssize_t write_format(const char* fmt, const Args&... args) {
    const auto bu
}

ssize_t write(const char* ptr) {

}

ssize_t write(const std::string& str) {

}


/* SocketStream implement */

SocketStream::SocketStream(
    int socket, time_t read_timeout_sec, time_t read_timeout_usec,
    time_t write_timeout_sec, time_t write_timeout_usec)
    : socket_(socket), read_timeout_sec_(read_timeout_sec), read_timeout_usec_(read_timeout_usec),
      write_timeout_sec_(write_timeout_sec), write_timeout_usec_(write_timeout_usec),
      read_buff_(read_buff_size_, 0) {}

bool SocketStream::is_readable() const {
    return util_network::check_socket_readable_by_select(socket_, read_timeout_sec_, read_timeout_usec_) > 0;
}

bool SocketStream::is_writeable() const {
    return util_network::check_socket_writeable_by_select(socket_, write_timeout_sec_, write_timeout_usec_) > 0
        && util_network::check_socket_alive(socket_);
}

ssize_t SocketStream::read(char* ptr, size_t size) {
    // size_t: typedef unsigned long size_t
    // ssize_t: typedef long ssize_t
    // size_t 相比 ssize_t 是大单位，因此我们取 ssize_t 的最大值作为可处理的最大值
    size = std::min(size, static_cast<size_t>((std::numeric_limits<ssize_t>::max())));
    // 如果当前 SocketStream 的缓存中是有可以读的数据，直接先拷贝返回
    if (read_buff_off_ < read_buff_content_size_) {
        auto remaining_size = read_buff_content_size_ - read_buff_off_;
        if (size <= remaining_size) {
            memcpy(ptr, read_buff_.data() + read_buff_off_, size);
            read_buff_off_ += size;
            return static_cast<ssize_t>(size);
        } else {
            memcpy(ptr, read_buff_.data() + read_buff_off_, remaining_size);
            read_buff_off_ += remaining_size;
            return static_cast<ssize_t>(remaining_size);
        }
    }
    // 如果不可读，则出错返回
    if (!is_readable()) {
        return -1;
    }
    // 此时 SocketStream 缓存中是没有数据的，从 socket 中读
    read_buff_off_ = 0;
    read_buff_content_size_ = 0;
    // 内部的缓存一般设置为 read_buff_size_ 大小
    if (size < read_buff_size_) {
        auto num = util_network::recv_socket(socket_, read_buff_.data(), read_buff_size_, 0);
        if (num <= 0) {
            // recv 调用出现错误
            return num;
        } else if (num <= static_cast<ssize_t>(size)) {
            // recv 得到的字节少于期望得到的字节
            memcpy(ptr, read_buff_.data(), static_cast<size_t>(num));
            return num;
        } else {
            // recv 得到的字节多于期望得到的字节
            // 读出来多于的字节保存起来
            memcpy(ptr, read_buff_.data(), size);
            read_buff_off_ = size;
            read_buff_content_size_ = static_cast<size_t>(num);
            return static_cast<ssize_t>(size);
        }
    } else {
        // 如果期望 recv 得到的字节大于缓存大小，则直接从 socket 中读，不进行缓存，因为没有意义
        return util_network::recv_socket(socket_, ptr, size, 0);
    }
}

ssize_t SocketStream::write(const char* ptr, size_t size) {
    if (!is_writeable()) {
        return -1;
    }
    return util_network::send_socket(socket_, ptr, size, 0);
}

void SocketStream::get_remote_ip_and_port(const std::string& ip, uint16_t port) const {

}

void SocketStream::get_local_ip_and_port(const std::string& ip, uint16_t port) const {

}

inline int SocketStream::get_socket_fd() const {
    return socket_;
}

/* SSLSocketStream implement */

SSLSocketStream::SSLSocketStream(int socket, SSL* ssl,
    time_t read_timeout_sec, time_t read_timeout_usec,
    time_t write_timeout_sec, time_t write_timeout_usec)
    : socket_(socket), ssl_(ssl),
      read_timeout_sec_(read_timeout_sec), read_timeout_usec_(read_timeout_usec),
      write_timeout_sec_(write_timeout_sec), write_timeout_usec_(write_timeout_usec) {
    SSL_clear_mode(ssl, SSL_MODE_AUTO_RETRY);
}

bool SSLSocketStream::is_readable() const {
    return util_network::check_socket_readable_by_select(socket_, read_timeout_sec_, read_timeout_usec_) > 0;
}

bool SSLSocketStream::is_writeable() const {
    return util_network::check_socket_writeable_by_select(socket_, write_timeout_sec_, write_timeout_usec_) > 0
        && util_network::check_socket_alive(socket_);
}

ssize_t SSLSocketStream::read(char* ptr, size_t size) {
    if (SSL_pending(ssl_) > 0) {
        return SSL_read(ssl_, static_cast<void*>(ptr), static_cast<int>(size));
    } else if (is_readable()) {
        auto res = SSL_read(ssl_, static_cast<void*>(ptr), static_cast<int>(size));
        if (res < 0) {
            auto err = SSL_get_error(ssl_, res);
            int n = 1000;
            while (--n >= 0 && err == SSL_ERROR_WANT_READ) {
                if (SSL_pending(ssl_) > 0) {
                    return SSL_read(ssl_, static_cast<void*>(ptr), static_cast<int>(size));
                } else if (is_readable()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    res = SSL_read(ssl_, static_cast<void*>(ptr), static_cast<int>(size));
                    if (res >= 0) {
                        return res;
                    }
                    err = SSL_get_error(ssl_, res);
                } else {
                    return -1;
                }
            }
        }
        return res;
    }
    return -1;
}

ssize_t SSLSocketStream::write(const char* ptr, size_t size) {
    if (is_writeable()) {
        auto handle_size = static_cast<int>(std::min<size_t>(size, (std::numeric_limits<int>::max()));
        auto res = SSL_write(ssl_, static_cast<const void*>(ptr), static_cast<int>(handle_size));
        if (res < 0) {
            auto err = SSL_get_error(ssl_, res);
            int n = 1000;
            while (--n >= 0 && err == SSL_ERROR_WANT_WRITE) {
                if (is_writeable()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    res = SSL_write(ssl_, static_cast<const void*>(ptr), static_cast<int>(handle_size));
                    if (res >= 0) {
                        return res;
                    }
                    err = SSL_get_error(ssl_, res);
                } else {
                    return -1;
                }
            }
        }
        return res;
    }
    return -1;
}

void SSLSocketStream::get_remote_ip_and_port(const std::string& ip, uint16_t port) const {
    
}

void SSLSocketStream::get_local_ip_and_port(const std::string& ip, uint16_t port) const {
    
}

inline int SSLSocketStream::get_socket_fd() const {
    return socket_;
}

/* BufferStream implement */

inline bool BufferStream::is_readable() const {
    return true;
}

inline bool BufferStream::is_writeable() const {
    return true;
}

inline ssize_t BufferStream::read(char* ptr, size_t size) {
    auto len_read = buffer_.copy(ptr, size, position);
    position += static_cast<size_t>(len_read);
    return static_cast<ssize_t>(len_read);
}

inline ssize_t BufferStream::write(const char* ptr, size_t size) {
    buffer_.append(ptr, size);
    return static_cast<ssize_t>(size);
}

inline void BufferStream::get_remote_ip_and_port(const std::string&, uint16_t) const {}

inline void BufferStream::get_local_ip_and_port(const std::string&, uint16_t) const {}

inline int BufferStream::get_socket_fd() const {
    return 0;
}

const std::string& BufferStream::get_buffer() const {
    return buffer_;
}

ssize_t write_request_headers_to_stream(std::shared_ptr<Stream> stream, const Headers& headers) {
    ssize_t write_len = 0;
    for (const auto& header : headers) {
        auto len = stream->write_format("%s %s\r\n", header.first.c_str(), header.second.c_str());
        if (len < 0) {
            return len;
        }
        write_len += len;
    }
    auto len = stream->write("\r\n");
    if (len < 0) {
        return len;
    }
    write_len += len;
    return write_len;
}

bool write_request_data_to_stream(std::shared_ptr<Stream> stream, const char* ptr, size_t size) {
    size_t offset = 0;
    while (offset < size) {
        auto len = stream->write(ptr+offset, size - offset);
        if (len < 0) {
            return false;
        }
        offset += len;
    }
    return true;
}

}  // namespace libevent_cpp
