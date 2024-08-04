
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>
#include <queue>
#include <utility>
#include <thread>
#include <mutex>
#include <condition_variable>

enum class Status {
  SUCCESS = 0,
  CLOSED,
  EMPTY,
  FULL
};

/*
注意 如果多个线程共享一个 blockqueue 进行push操作
需要等到所有线程的push操作都结束了之后
才能进行close操作
close 操作需要notify_all 所有的线程，原因是由于有的线程
在push测或者pop测等待 condition_variable 的信号
*/
template<typename T>
class BlockQueue {
  public:
    explicit BlockQueue(int capacity = 0): capacity_(capacity) {
      if (capacity_ <= 0) {
        capacity_ = 0;
      }
      is_close_ = false;
    }
    BlockQueue(const BlockQueue&) = delete;
    BlockQueue& operator=(const BlockQueue&) = delete;

    ~BlockQueue() {
      {
        std::lock_guard<std::mutex> lock(mtx_);
        assert(queue_.empty());
        assert(is_close_);
      }
      cv_not_empty_.notify_all();
      cv_not_full_.notify_all();
    }

    template<typename ...Args>
    Status push(Args&& ...args) {
      {
        std::unique_lock<std::mutex> lock(mtx_);
        if (is_close_) {
          return Status::CLOSED;
        }
        if (capacity_ > 0) {
          cv_not_full_.wait(lock, [this](){
            return is_close_ || static_cast<int>(queue_.size()) < capacity_;
          });
        }
        queue_.emplace(std::forward<Args>(args)...);
      }
      cv_not_empty_.notify_one();
      return Status::SUCCESS;
    }

    Status pop(T* ret) {
      Status status;
      {
        std::unique_lock<std::mutex> lock(mtx_);
        if (is_close_ && queue_.empty()) {
          return Status::CLOSED;
        }
        cv_not_empty_.wait(lock, [this](){
          return is_close_ || !queue_.empty();
        });
        if (!queue_.empty()) {
          *ret = std::move(queue_.front());
          queue_.pop();
          status = Status::SUCCESS;
        } else {
          status = Status::EMPTY;
        }
      }
      if (status == Status::SUCCESS) {
        cv_not_full_.notify_one();
      }
      return status;
    }

    template<typename ...Args>
    Status try_push(Args&& ...args) {
      {
        std::unique_lock<std::mutex> lock(mtx_);
        if (is_close_) {
          return Status::CLOSED;
        }
        if (queue_.size() >= capacity_) {
          return Status::FULL;
        }
        queue_.emplace(std::forward<Args>(args)...);
      }
      cv_not_empty_.notify_one();
      return Status::SUCCESS;
    }

    Status try_pop(T* ret) {
      {
        std::unique_lock<std::mutex> lock(mtx_);
        if (is_close_ && queue_.empty()) {
          return Status::CLOSED;
        }
        if (queue_.empty()) {
          return Status::EMPTY;
        }
        *ret = std::move(queue_.front());
        queue_.pop();
      }
      cv_not_full_.notify_one();
      return Status::SUCCESS;
    }

    void close() {
      std::lock_guard<std::mutex> lock(mtx_);
      is_close_ = true;
      cv_not_empty_.notify_all();
      cv_not_full_.notify_all();
    }

    bool is_close() {
      std::lock_guard<std::mutex> lock(mtx_);
      return is_close_;
    }

  private:
    int capacity_;
    bool is_close_;
    std::mutex mtx_;
    std::queue<T> queue_;
    std::condition_variable cv_not_full_;
    std::condition_variable cv_not_empty_;
};


std::atomic_int num{0};

void test0() {
  const int push_thread_num = 128;
  const int pop_thread_num = 2;
  std::shared_ptr<BlockQueue<int>> queue = std::make_shared<BlockQueue<int>>(100);
  std::vector<std::thread> threads;
  for (int i = 0; i < push_thread_num; i++) {
    threads.emplace_back([](std::shared_ptr<BlockQueue<int>> queue) {
      for (int i = 0; i < 200; i++) {
        queue->push(i);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
      std::this_thread::sleep_for(std::chrono::seconds(5));
      queue->close();
    }, queue);
  }
  for (int i = 0; i < pop_thread_num; i++) {
    threads.emplace_back([](std::shared_ptr<BlockQueue<int>> queue) {
      Status status = Status::SUCCESS;
      while(status != Status::CLOSED) {
        int ret;
        status = queue->pop(&ret);
        if (status == Status::SUCCESS) {
          num++;
          std::cout << "currnet num: " << num << std::endl;
         }
        std::this_thread::sleep_for(std::chrono::microseconds(5));
      }
    }, queue);
  }
  for (std::size_t i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
}

int main() {
  std::cout << "============== test 0 ============" << std::endl;
  test0();
}