#ifndef __BLOCKING_COUNTER__
#define __BLOCKING_COUNTER__


#include <mutex>
#include <condition_variable>
class BlockingCounter final {
public:
    BlockingCounter(int64_t count): cnt_(count) {}
    BlockingCounter() = delete;
    BlockingCounter(const BlockingCounter& other) = delete;
    BlockingCounter& operator=(const BlockingCounter& other) = delete;
    BlockingCounter(BlockingCounter&& other) = delete;
    BlockingCounter& operator=(BlockingCounter&& other) = delete;

    int64_t increase();
    int64_t decrease();
    void WaitForeverUntilCntEqualZero();


private:
    std::mutex mtx_;
    std::condition_variable cv_;
    int64_t cnt_;
};

#endif