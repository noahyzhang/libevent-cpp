#include <queue>
#include <mutex>

template <typename T>
class concurrent_queue {
 public:  
    void push(T const& value) {
        std::unique_lock<std::mutex> lock(mutex_); 
        queue_.push(value); 
    }
    bool pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = queue_.front();
        queue_.pop();
        return true;  
    }
    bool empty() {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

 private:  
    std::queue<T> queue_;
    std::mutex mutex_;
};