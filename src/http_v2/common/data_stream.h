// Copyright 2022 Tencent LLC
// Author: noahyzhang

#include <stdint.h>
#include <stdlib.h>
#include <openssl/ssl.h>
#include <string>
#include <vector>
#include <memory>
#include "http_v2/common/common.h"

#pragma once

namespace libevent_cpp {

// 读写的流
class Stream {
public:
    virtual ~Stream() = default;

    virtual bool is_readable() const = 0;
    virtual bool is_writeable() const = 0;

    virtual ssize_t read(char* ptr, size_t size) = 0;
    virtual ssize_t write(const char* ptr, size_t size) = 0;
    virtual void get_remote_ip_and_port(const std::string& ip, uint16_t port) const = 0;
    virtual void get_local_ip_and_port(const std::string& ip, uint16_t port) const = 0;
    virtual int get_socket_fd() const = 0;

    template <typename... Args>
    ssize_t write_format(const char* fmt, const Args&... args);
    ssize_t write(const char* ptr);
    ssize_t write(const std::string& str);
};

// socket 读写的流
class SocketStream : public Stream {
public:
    SocketStream() = delete;
    SocketStream(
        int socket, time_t read_timeout_sec, time_t read_timeout_usec,
        time_t write_timeout_sec, time_t write_timeout_usec);
    ~SocketStream() override;

public:
    bool is_readable() const override;
    bool is_writeable() const override;

    ssize_t read(char* ptr, size_t size) override;
    ssize_t write(const char* ptr, size_t size) override;
    void get_remote_ip_and_port(const std::string& ip, uint16_t port) const override;
    void get_local_ip_and_port(const std::string& ip, uint16_t port) const override;
    int get_socket_fd() const override;

private:
    int socket_ = -1;
    time_t read_timeout_sec_ = 0;
    time_t read_timeout_usec_ = 0;
    time_t write_timeout_sec_ = 0;
    time_t write_timeout_usec_ = 0;

    // 缓存，保存上次未读完的数据
    std::vector<char> read_buff_;
    // 上次读缓存读到的位置
    size_t read_buff_off_ = 0;
    // 上次缓存总共保存的数据
    size_t read_buff_content_size_ = 0;
    // 缓存默认大小
    static const size_t read_buff_size_ = 1024 * 4;
};

// SSL socket 读写的流
class SSLSocketStream : public Stream {
public:
    SSLSocketStream() = delete;
    SSLSocketStream(
        int socket, SSL* ssl, time_t read_timeout_sec, time_t read_timeout_usec,
        time_t write_timeout_sec, time_t write_timeout_usec);
    ~SSLSocketStream() override = default;

public:
    bool is_readable() const override;
    bool is_writeable() const override;

    ssize_t read(char* ptr, size_t size) override;
    ssize_t write(const char* ptr, size_t size) override;
    void get_remote_ip_and_port(const std::string& ip, uint16_t port) const override;
    void get_local_ip_and_port(const std::string& ip, uint16_t port) const override;
    int get_socket_fd() const override;

private:
    int socket_;
    SSL* ssl_;
    time_t read_timeout_sec_;
    time_t read_timeout_usec_;
    time_t write_timeout_sec_;
    time_t write_timeout_usec_;
};

// 用于做数据缓存的流
class BufferStream : public Stream {
public:
    BufferStream() = default;
    ~BufferStream() override = default;

public:
    bool is_readable() const override;
    bool is_writeable() const override;

    ssize_t read(char* ptr, size_t size) override;
    ssize_t write(const char* ptr, size_t size) override;
    void get_remote_ip_and_port(const std::string& ip, uint16_t port) const override;
    void get_local_ip_and_port(const std::string& ip, uint16_t port) const override;
    int get_socket_fd() const override;

    const std::string& get_buffer() const;

private:
    std::string buffer_;
    size_t position = 0;
};

ssize_t write_request_headers_to_stream(std::shared_ptr<Stream> stream, const Headers& headers);
bool write_request_data_to_stream(std::shared_ptr<Stream> stream, const char* ptr, size_t size);
// bool write_content(Stream& stream, const ContentProvider& content_provider, size_t offset, size_t length, )


}  // namespace libevent_cpp

