#include <assert.h>
#include "block_counter.h"

int64_t BlockingCounter::increase() {
    std::lock_guard<std::mutex> lk(mtx_);
    assert(cnt_ > 0);
    cnt_ += 1;
    return cnt_;
}

int64_t BlockingCounter::decrease() {
    std::lock_guard<std::mutex> lk(mtx_);
    cnt_ -= 1;
    if (cnt_ == 0) {
        cv_.notify_all();
    }
    return cnt_;
}

void BlockingCounter::WaitForeverUntilCntEqualZero() {
    std::unique_lock<std::mutex> lck(mtx_);
    cv_.wait(lck, [this]() { return cnt_ == 0; });
}