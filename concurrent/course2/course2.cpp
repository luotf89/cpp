#include <chrono>
#include <thread>
#include <iostream>
#include <utility>

/*
仿函数做作为线程函数

根据源码显示 thread 内部有个 id 结构体保留着 _M_thread 数据成员 该数据成员的类型其实就是pthread_t
id 的 默认无参构造函数 _M_thread = 0

如果 _M_thread != 0 这线程 为joinable
线程的join 函数 会做如下事情：
1、判断前线程的_M_thread != 0  空的线程不能join
2、判断当前调用join的 _M_thread 是否等于 被调用的线程的 _M_thread 自己不能join自己
3、等待被调用线程执行完毕
4、将被调用线程的 _M_thread 设置为 0， 即被join后的线程不能被二次join
*/
void test1() {
  std::thread t1;
  std::thread t2(
    [](){
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "t2" << std::endl;
    }
  );
  std::thread t3(
    []() {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::cout << "t3" << std::endl;
    }
  );
  t1 = std::move(t2);
  if (t1.joinable()) {
    std::cout << "t1.joinable" << std::endl;
    t1.join();
  }
  // 该段代码不会被执行到， 因为 t2的所有权已经被t1 状态不会是 
  if (t2.joinable()) {
    std::cout << "t2.joinable" << std::endl;
    t2.join();
  }
  if (t3.joinable()) {
    t3.join();
  }
  t3 = std::move(t2);
  // 该段代码不会被执行 因为 t2的 _M_thread 为 0
  if (t3.joinable()) {
    std::cout << "t3.joinable" << std::endl;
    t3.join();
  }
  
  std::this_thread::sleep_for(std::chrono::seconds(3));
}

/*
join_thread 线程自动汇合
*/
class join_thread {
  public:
    join_thread() {}
    explicit join_thread(std::thread& t): t_(std::move(t)) {}
    explicit join_thread(std::thread&& t): t_(std::move(t)) {}
    template<typename Func, typename ...Args>
    explicit join_thread(Func&& func, Args&& ...args): t_(std::forward<Func>(func), std::forward<Args>(args)...) {}
    join_thread(const join_thread&) = delete;
    join_thread& operator=(const join_thread&) = delete;
    ~join_thread() {
      if (joinable()) {
        join();
      }
    }
    join_thread(join_thread&& other) {
      if (joinable()) {
        join();
      }
      swap(other);
    }
    join_thread& operator=(join_thread&& other) {
      if (this != &other) {
        if (joinable()) {
          join();
        }
        swap(other);
      }
      return *this;
    }
    join_thread& swap(join_thread& other) {
      t_.swap(other.t_);
      return *this;
    }

    bool joinable() {
      return t_.joinable();
    }

    void join() {
      t_.join();
    }

    void detach() {
      t_.detach();
    }
    std::thread::id get_id() {
      return t_.get_id();
    }

    std::thread& as_thread() {
      return t_;
    }

    const std::thread& as_thread() const {
      return t_;
    }

  private:
    std::thread t_;
};
void test2() {
  std::thread t1;
  join_thread tj1(t1);

  std::thread t2;
  join_thread tj2(std::move(t2));

  join_thread tj3(std::move(tj1));

  tj2 = std::move(tj3);

  tj1.swap(tj2);

}

int main() {
  test1();
  test2();
  return 0;
}