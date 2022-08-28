#include "thread_pool.h"

void libevent_cpp::thread_pool::set_thread(int i) {
    std::shared_ptr<std::atomic<bool>> flag(flags_[i]);
    auto f = [this, i, flag]() {
        std::atomic<bool>& internal_flag = *flag;
        std::function<void(int id)>* func;
        bool is_pop = queue_.pop(func);
        while (true) {
            while (is_pop) {
                std::unique_ptr<std::function<void(int id)>> f(func);
                (*f)(i);
                if (internal_flag) {
                    return; 
                } else {
                    is_pop = queue_.pop(func); 
                }
            }
            // 如果 queue_ 为空，则阻塞等待，当 push 进一个 func 时，会唤醒条件变量
            std::unique_lock<std::mutex> lock(mutex_);
            ++waiting_threads_;
            cv_.wait(lock, [this, &func, &is_pop, &internal_flag]() {
                is_pop = queue_.pop(func);
                return is_pop || is_done_ || internal_flag;
            });
            --waiting_threads_;
            if (!is_pop) {
                return; 
            }
        }
    };
    threads_[i].reset(new std::thread(f));
}

void libevent_cpp::thread_pool::reset_thread_num(int thread_num) {
    if (!is_stop_ && !is_done_) {
        int old_thread_num = static_cast<int>(threads_.size());
        // 如果新的线程数等于原来的线程数，不需要扩缩容
        if (old_thread_num == thread_num) {
            return;
        } else if (old_thread_num < thread_num) {
            // 扩容的情况
            threads_.resize(thread_num);
            flags_.resize(thread_num); 
            // 设置扩容的那几个线程
            for (int i = old_thread_num; i < thread_num; i++) {
                flags_[i] = std::make_shared<std::atomic<bool>>(false);
                set_thread(i); 
            }
        } else {
            // 缩容的情况，把将要被缩掉的线程设置 flag，使其退出 
            for (int i = old_thread_num -1; i >= thread_num; --i) {
                *flags_[i] = true;
                threads_[i]->detach();
            }
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.notify_all();
            }
            threads_.resize(thread_num);
            flags_.resize(thread_num);
        }
    }
}

void libevent_cpp::thread_pool::clear_queue() {
    std::function<void(int id)>* func;
    while (queue_.pop(func)) {
        delete func; 
    }
}

std::function<void(int)> libevent_cpp::thread_pool::pop() {
    std::function<void(int id)>* func = nullptr;
    queue_.pop(func);
    // 在返回时，即使发生异常也要释放此 func 
    std::unique_ptr<std::function<void(int id)>> tmp_func(func);
    std::function<void(int)> res;
    if (func) {
        res = *func;
    }
    return res; 
}

void libevent_cpp::thread_pool::stop(bool is_wait = false) {
    if (!is_wait) {
        if (is_stop_) {
            return;
        }
        is_stop_ = true;
        for (int i = 0, n = get_thread_num(); i < n; ++i) {
            *flags_[i] = true; 
        }
        clear_queue();
    } else {
        if (is_done_ || is_stop_) {
            return; 
        }
        is_done_ = true; 
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.notify_all();
    }
    for (int i = 0; i < get_thread_num(); ++i) {
        if (threads_[i]->joinable()) {
            threads_[i]->join();
        }
    }
    clear_queue();
    threads_.clear();
    flags_.clear();
}
