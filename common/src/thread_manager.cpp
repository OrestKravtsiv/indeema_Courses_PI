#include "thread_manager.hpp"


// Template implementation must be in header
template<typename T>
void ThreadSafeQueue<T>::push(T value) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
    } // Lock released here
    
    // Notify one waiting thread that data is available
    condVar_.notify_one();
}

template<typename T>
std::optional<T> ThreadSafeQueue<T>::tryPop() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (queue_.empty()) {
        return std::nullopt;
    }
    
    T value = std::move(queue_.front());
    queue_.pop();
    return value;
}

template<typename T>
T ThreadSafeQueue<T>::waitAndPop() {
    std::unique_lock<std::mutex> lock(mutex_);
    
    // Wait until queue is not empty
    // This releases the lock while waiting and reacquires it when notified
    condVar_.wait(lock, [this] { return !queue_.empty(); });
    
    T value = std::move(queue_.front());
    queue_.pop();
    return value;
}

template<typename T>
bool ThreadSafeQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}