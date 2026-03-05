#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class ThreadSafeQueue {
public: 
    ThreadSafeQueue() = default;
    
    // Delete copy constructor and assignment (queues shouldn't be copied)
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
    
    // Push an item to the queue (thread-safe)
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_.push(std::move(value));
        } // Lock released here
        
        // Notify one waiting thread that data is available
        condVar_.notify_one();
    }
    
    // Try to pop an item (non-blocking)
    // Returns std::nullopt if queue is empty
    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (queue_.empty()) {
            return std::nullopt;
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    
    // Pop an item (blocking)
    // Waits until an item is available
    T waitAndPop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until queue is not empty
        // This releases the lock while waiting and reacquires it when notified
        condVar_.wait(lock, [this] { return !queue_.empty(); });
        
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
    
    // Check if queue is empty (snapshot, may change immediately)
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }
    
    // Get queue size (snapshot, may change immediately)
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;              // Protects the queue
    std::condition_variable condVar_;       // For blocking wait
    std::queue<T> queue_;                   // The actual queue
};


#endif // THREAD_SAFE_QUEUE_HPP
