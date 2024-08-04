// block_queue.h
#ifndef __BLOCK_QUEUE_H
#define __BLOCK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <assert.h>

enum class Status { kChannelStatusSuccess = 0, kChannelStatusErrorClosed };

template<typename T>
class BlockQueue {
public:
    BlockQueue():is_closed_(false) {};
    BlockQueue(const BlockQueue& ) = delete;
    BlockQueue(BlockQueue&& ) = delete;
    BlockQueue operator=(const BlockQueue& ) = delete;
    BlockQueue operator=(BlockQueue&& ) = delete;
    bool empty() {
        std::lock_guard<std::mutex> lock(mtx_);
        return q_.empty();
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(mtx_);
        return q_.size();
    }
    template<typename ...ARGS>
    Status emplace(ARGS&& ...args) {
        bool notify;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (is_closed_) {
                return Status::kChannelStatusErrorClosed;
            }
            notify = q_.empty();
            q_.emplace(std::forward<ARGS>(args)...);
        }
        if (notify) {
            cv_.notify_one();
        }
        return Status::kChannelStatusSuccess;
    }
    template<typename ...ARGS>
    Status push(ARGS&& ...args) {
        bool notify;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (is_closed_) {
                return Status::kChannelStatusErrorClosed;
            }
            notify = q_.empty();
            q_.emplace(std::forward<ARGS>(args)...);
        }
        if (notify) {
            cv_.notify_one();
        }
        return Status::kChannelStatusSuccess;
    }
    Status pop(T* item) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this](){return !q_.empty() || is_closed_;});
        if (q_.empty()) {
            return Status::kChannelStatusErrorClosed;
        };
        *item = std::move(q_.front());
        q_.pop();
        return Status::kChannelStatusSuccess;
    }

    Status pop(std::queue<T>* items) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this](){return !q_.empty() || is_closed_;});
        if (q_.empty()) {
            return Status::kChannelStatusErrorClosed;
        }
        while(!q_.empty()) {
            items->push(std::move(q_.front()));
            q_.pop();
        }
        return Status::kChannelStatusSuccess;
    }

    void close() {
        std::unique_lock<std::mutex> lock(mtx_);
        is_closed_ = true;
        cv_.notify_all();

    }
private:
    bool is_closed_;
    std::mutex mtx_;
    std::queue<T> q_;
    std::condition_variable cv_;
};

#endif // endif __BLOCK_QUEUE_H