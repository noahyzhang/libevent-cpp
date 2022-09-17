// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <unistd.h>
#include <utility>
#include <vector>
#include <queue>
#include <memory>

namespace libevent_cpp {

template <typename Alloc>
struct alloc_deleter {
 public:
    explicit alloc_deleter(Alloc* alloc) : alloc(alloc) {}
    void operator()(typename Alloc::pointer ptr) const {
        alloc->deallocate(ptr);
    }
 private:
    Alloc* alloc;
};

template <typename T, size_t BLOCK_SIZE = 4096>
class pool {
using Deleter = alloc_deleter<pool<T>>;

 public:
    pool() {
        size_per_block_ = static_cast<int>(BLOCK_SIZE / sizeof(T));
        cur_index_ = size_per_block_;
        begin_ = nullptr;
    }
    ~pool() = default;

    template <typename... Args>
    inline std::unique_ptr<T, Deleter> allocate_unique(Args &&... args) {
        auto ptr = allocate();
        new (ptr) T(std::forward<Args>(args)...);
        return std::unique_ptr<T, Deleter>(ptr, Deleter(this));
    }

    inline T* allocate() {
        if (freeQueue_.empty()) {
            if (cur_index_ >= size_per_block_ || begin_ == nullptr) {
                allocate_block();
            }
            auto ptr = begin_ + cur_index_;
            cur_index_++;
            return ptr;
        } else {
            auto ptr = freeQueue_.front();
            freeQueue_.pop();
            return ptr;
        }
    }

    inline void deallocate(T* ptr) {
        freeQueue_.push(ptr);
    }

    void allocate_block() {
        void* new_block = operator new(BLOCK_SIZE);
        cur_index_ = 0;
        blocks_.push_back(new_block);
        begin_ = reinterpret_cast<T*>(new_block);
    }

 public:
    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;
    typedef const T *const_pointer;
    typedef const T &const_reference;
    typedef std::unique_ptr<T, Deleter> unique_ptr_type;

 private:
    std::vector<void*> blocks_;
    std::queue<T*> freeQueue_;
    T* begin_;
    int size_per_block_;
    int cur_index_;
};

}  // namespace libevent_cpp
