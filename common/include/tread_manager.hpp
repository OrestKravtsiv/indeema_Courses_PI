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
    void push(T value);
    
    // Try to pop an item (non-blocking)
    // Returns std::nullopt if queue is empty
    std::optional<T> tryPop();
    
    // Pop an item (blocking)
    // Waits until an item is available
    T waitAndPop();
    
    // Check if queue is empty (snapshot, may change immediately)
    bool empty() const;
    
    // Get queue size (snapshot, may change immediately)
    size_t size() const;

private:
    mutable std::mutex mutex_;              // Protects the queue
    std::condition_variable condVar_;       // For blocking wait
    std::queue<T> queue_;                   // The actual queue
};


#endif // THREAD_SAFE_QUEUE_HPP
