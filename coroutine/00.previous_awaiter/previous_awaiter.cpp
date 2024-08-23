#include <chrono>
#include "../utils/debug.h"
#include "../utils/awaiter.h"
#include "../utils/promise.h"


template <class T>
struct Task {
    using promise_type = Promise<T>;

    Task(std::coroutine_handle<promise_type> coroutine) noexcept
        : handle_(coroutine) {}

    Task(Task &&) = delete;

    ~Task() {
        handle_.destroy();
    }

    struct Awaiter {
        bool await_ready() const noexcept { return false; }

        std::coroutine_handle<promise_type> await_suspend(std::coroutine_handle<> coroutine) const noexcept {
            debug(), " await_suspend: handle_: ", handle_.address(), " coroutine: ", coroutine.address();
            handle_.promise().prev_handle_ = coroutine;
            return handle_;
        }

        T await_resume() const {
            return handle_.promise().result();
        }

        std::coroutine_handle<promise_type> handle_;
    };

    auto operator co_await() const noexcept {
        return Awaiter(handle_);
    }

    std::coroutine_handle<promise_type> handle_;
};

Task<std::string> baby() {
    debug(), "baby";
    co_return "aaa\n";
}

Task<double> world() {
    debug(), "world";
    co_return 3.14;
}

Task<int> hello() {
    auto baby_t = baby();
    debug(), "baby_t: ", baby_t.handle_.address();
    auto ret = co_await baby_t;
    debug(), ret;
    // baby_t.mCoroutine.resume();
    auto world_t = world();
    debug(), "world_t: ", world_t.handle_.address();
    int i = (int)co_await world_t;
    debug(), "hello得到world结果为", i;
    co_return i + 1;
}

int main() {
    debug(), "main即将调用hello";
    auto t = hello();
    debug(), "hello handle_: ", t.handle_.address();
    debug(), "main调用完了hello"; // 其实只创建了task对象，并没有真正开始执行
    while (!t.handle_.done()) {
        t.handle_.resume();
        debug(), "main得到hello结果为",
            t.handle_.promise().result();
    }
    return 0;
}