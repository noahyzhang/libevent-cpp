#ifndef LIBEVENT_CPP_BUFFER_H
#define LIBEVENT_CPP_BUFFER_H

#include <memory> 
#include <stdlib.h>

namespace libevent_cpp {

// 起初默认的 buffer 大小
#define DEFAULT_BUFFER_SIZE 128 
// 单次从文件读取的最大字节数
#define DEFAULT_ONCE_READ_FILE_MAX_BYTES 4096 

class buffer {
private: 
    unsigned char* origin_buf_;
    unsigned char* buf_;
    size_t total_len_ = 0; 
    size_t cur_len_ = 0; 
    size_t start_index_ = 0;

public:
    buffer() : total_len_(DEFAULT_BUFFER_SIZE) {
        origin_buf_ = new unsigned char[total_len_];
        buf_ = origin_buf_;
    }
    ~buffer() {
        delete origin_buf_;
    }
    // 重置
    void reset();
    // 重新设置大小
    void resize(size_t n);
    // 读取一行，分割符可以为：'\r\n', '\n\r', '\r', '\n' 
    std::string readline();
    
    // 从文件中读取数据
    int read_file(size_t fd, int size);
    // 将数据写入文件
    size_t write_file(size_t fd);

    // 尾插
    int push_back(void* data, size_t data_len);
    int push_back_buffer(std::unique_ptr<buffer>& buf, size_t buf_len);
    inline int push_back_string(const std::string& str) {
        return push_back((void*)str.c_str(), str.size());
    }

    // 头删
    size_t pop_front(void* data, size_t data_len);

    // 查询
    // unsigned char* find(unsigned char* what, size_t len);
    // inline unsigned char* find_string(const std::string& what) {
    //     return find((unsigned char*)what.c_str(), what.length());
    // }

    inline size_t get_cur_length() const { return cur_len_; } 

    inline const char* get_data() const {
        return (const char*)buf_; 
    }

private:
    // 扩展 buffer，长度不够时，扩展空间
    int expand_buffer(size_t data_len);
    // 删除一定长度的字符数组，头删
    void remove_front(size_t len);
    // 复位 buffer，将字符数组的前面的已经被删除的部分补齐. [start_index_, start_index_ + cur_len_] => [0, cur_len_] 
    void reset_buffer();

};

} // namespace libevent_cpp

#endif // LIBEVENT_CPP_BUFFER_