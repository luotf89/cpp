// threads_pool.h
#ifndef __THREADS_POOL_H
#define __THREADS_POOL_H

#include <atomic>
#include <thread>
#include <iostream>
#include <functional>
#include <future>
#include "block_queue.h"
#include "singleton.h"

class ThreadsPool {

    friend Singleton<ThreadsPool>;
private:
    explicit ThreadsPool(int n): index_(0), threads_num_(n), channels_(n){
        for (size_t i = 0; i < threads_num_; i++ ) {
            BlockQueue<std::function<void()>>* chann = &(channels_.at(i));
            std::thread th([chann, this] {
                // std::function<void()> task;
                std::queue<std::function<void()>> tasks;
                while(chann->pop(&tasks) == Status::kChannelStatusSuccess) {
                    // std::cout << "hello world!" <<  threads_num_ << std::endl;
                    // task();
                    while(!tasks.empty()) {
                        tasks.front()();
                        tasks.pop();
                    }
                }
            });
            threads_.push_back(std::move(th));
        }
        std::cout << "thread_num: " << threads_num_ << std::endl;
    }
    ThreadsPool():ThreadsPool(std::thread::hardware_concurrency()){}
public:
    template<typename Func, typename ...Args>
    auto submit(Func&& func, Args&& ...args) {
        using Ret = decltype(func(args...));
        // auto task = std::make_shared<std::packaged_task<Ret()>>(
        //     [func = std::forward<Func>(func), ...args = std::forward<Args>(args)](){
        //         return func(args...);
        //     }
        // ); // lambda 值捕获变参 需要c++20 才能支持
        // auto task = std::make_shared<std::packaged_task<Ret()>>(
        //     [func = std::forward<Func>(func), &...args = std::forward<Args>(args)](){
        //         return func(args...);
        //     }
        // ); // lambda 引用捕获变参 需要c++20 才能支持
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        ); 
        std::future<Ret> res = task->get_future();
        channels_[index_%threads_num_].push(
            [task](){
                (*task)();
            }
        );
        index_++;
        return res;
    }

    ~ThreadsPool() {
        for(int i = 0; i < threads_num_; i++) {
            channels_[i].close();
            if (threads_[i].joinable()) {
                threads_[i].join();
            }
        } 
    }

private:
    size_t index_;
    const size_t threads_num_;
    std::vector<BlockQueue<std::function<void()>>> channels_;
    std::vector<std::thread> threads_; 
};

#endif // endif __THREADS_POOL_H