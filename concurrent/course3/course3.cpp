#include <cassert>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <stack>
#include <thread>
#include <iostream>
#include <memory>
#include <vector>

/*
有时候我们可以将对共享数据的访问和修改聚合到一个函数，在函数内加锁保证数据的安全性。但是对于读取类型的操作，
即使读取函数是线程安全的，但是返回值抛给外边使用，存在不安全性。比如一个栈对象，我们要保证其在多线程访问的时候是安全的，
可以在判断栈是否为空，判断操作内部我们可以加锁，但是判断结束后返回值就不在加锁了，就会存在线程安全问题。
比如我定义了如下栈, 对于多线程访问时判断栈是否为空，此后两个线程同时出栈，可能会造成崩溃。

pop采用shared_ptr的作用是由于 shared_ptr 可以设置空状态，
如果在外部查询empty 状态，如果不为空的话 直接调用pop函数，而pop函数没有查询 empty状态 会造成coredump
*/
template <typename T>
class safe_stack{
  public:
    safe_stack() = default;
    safe_stack(const safe_stack&) = delete;
    safe_stack(safe_stack&&) = delete;
    safe_stack& operator=(const safe_stack&) = delete;
    safe_stack& operator=(safe_stack&&) = delete;
    void push(T elem) {
      std::lock_guard<std::mutex> lock(mtx_);
      stack_.push(elem);
    }

    std::shared_ptr<T> pop() {
      std::lock_guard<std::mutex> lock(mtx_);
      if (stack_.empty()) {
        return nullptr;
      }
      std::shared_ptr<T> ret = std::make_shared<T>(stack_.top());
      stack_.pop();
      return ret;
    }
  private:
    std::mutex mtx_;
    std::stack<T> stack_;
};
void test1() {

  safe_stack<int> gss;
  std::thread t1(
    [](safe_stack<int>& gss){
      for(int i = 0; i < 10000000; i++) {
        gss.push(i);
      }
    },
    std::ref(gss)
  );
  std::thread t2(
    [](safe_stack<int>& gss) {
      for(int i = 0; i < 10000000; i++) {
        auto ret = gss.pop();
        if (ret) {
          std::cout << "thread 2 pop: " << *ret << std::endl;
        }
      }
    },
    std::ref(gss)
  );
  std::thread t3(
    [](safe_stack<int>& gss) {
      for(int i = 0; i < 10000000; i++) {
        auto ret = gss.pop();
        if (ret) {
          std::cout << "thread 3 pop: " << *ret << std::endl;
        }
      }
    },
    std::ref(gss)
  );
  t1.join();
  t2.join();
  t3.join();
}


/*
死锁一般是由于调运顺序不一致导致的，比如两个线程循环调用。当线程1先加锁A，再加锁B，而线程2先加锁B，再加锁A。
那么在某一时刻就可能造成死锁。比如线程1对A已经加锁，线程2对B已经加锁，那么他们都希望彼此占有对方的锁，又不释放自己占有的锁导致了死锁。
*/
void test2() {
  std::mutex mtx1;
  std::mutex mtx2;
  // std::thread t1([&](){
  //   for(int i = 0; i < 100; i++) {
  //     mtx1.lock();
  //     std::this_thread::sleep_for(std::chrono::microseconds(1));
  //     mtx2.lock();
  //     std::this_thread::sleep_for(std::chrono::microseconds(2));
  //     mtx2.unlock();
  //     mtx1.unlock();
  //   }
  // });
  // std::thread t2([&](){
  //   for(int i = 0; i < 100; i++) {
  //     mtx2.lock();
  //     std::this_thread::sleep_for(std::chrono::microseconds(1));
  //     mtx1.lock();
  //     std::this_thread::sleep_for(std::chrono::microseconds(2));
  //     mtx1.unlock();
  //     mtx2.unlock();
  //   }
  // });
  // t1.join();
  // t2.join();

  //上面代码会导致死锁，解决方法，将每一个mutex操作在封在一个函数内这样就不会出现死锁
  auto func1 = [&]() {
    mtx1.lock();
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    mtx1.unlock();
  };
  auto func2 = [&](){
    mtx2.lock();
    std::this_thread::sleep_for(std::chrono::microseconds(2));
    mtx2.unlock();
  };
  std::thread t1([&](){
    for(int i = 0; i < 100; i++) {
      func1();
      func2();
    }
  });
  std::thread t2([&](){
    for(int i = 0; i < 100; i++) {
      func2();
      func1();
    }
  });
  t1.join();
  t2.join();
}


/*
当我们无法避免在一个函数内部使用两个互斥量，并且都要解锁的情况，那我们可以采取同时加锁的方式
*/
void test3() {
  class A {
    public:
      A() = default;
      // //这种方式会用问题，原因是由于没有同时上锁导导致死锁
      // // 比如线程1执行 a.swap(b) 线程2执行 b.swap(a) 造成死锁
      // A& swap(A& other) {
      //   {
      //     std::lock_guard<std::mutex> lock1(mtx_);
      //     std::lock_guard<std::mutex> lock2(other.mtx_);
      //     data_.swap(other.data_);
      //   }
      //   return *this;
      // }
      A& safe_swap1(A& other) {
        {
          std::lock(mtx_, other.mtx_);
          std::lock_guard<std::mutex> lock1(mtx_, std::adopt_lock); // 领养锁
          std::lock_guard<std::mutex> lock2(other.mtx_, std::adopt_lock); // 领养锁
          data_.swap(other.data_);
        }
        return *this;
      }
      A& safe_swap2(A& other) {
        {
          std::scoped_lock<std::mutex, std::mutex> lock(mtx_, other.mtx_);
          data_.swap(other.data_);
        }
        return *this;
      }
    private:
      std::mutex mtx_;
      std::vector<int> data_;
  };
  A a, b;
  //上面代码会导致死锁，解决方法，将每一个mutex操作在封在一个函数内这样就不会出现死锁
  std::thread t1([&](){
    for(int i = 0; i < 1000000; i++) {
      a.safe_swap1(b);
      a.safe_swap2(b);
    }
  });
  std::thread t2([&](){
    for(int i = 0; i < 1000000; i++) {
      b.safe_swap1(a);
      b.safe_swap2(a);
    }
  });
  t1.join();
  t2.join();
}

/*
现实开发中常常很难规避同一个函数内部加多个锁的情况，我们要尽可能避免循环加锁，
所以可以自定义一个层级锁，保证实际项目中对多个互斥量加锁时是有序的。
*/
class hierarchical_mutex {
  public:
    hierarchical_mutex() = delete;
    hierarchical_mutex(const int hierarchical_value): hierarchical_value_(hierarchical_value){}
    hierarchical_mutex(const hierarchical_mutex&) = delete;
    hierarchical_mutex& operator=(const hierarchical_mutex&) = delete;
    hierarchical_mutex(hierarchical_mutex&&) = delete;
    hierarchical_mutex& operator=(hierarchical_mutex&&) = delete;
    // //这种方式会用问题，原因是由于没有同时上锁导导致死锁
    // // 比如线程1执行 a.swap(b) 线程2执行 b.swap(a) 造成死锁
    // A& swap(A& other) {
    //   {
    //     std::lock_guard<std::mutex> lock1(mtx_);
    //     std::lock_guard<std::mutex> lock2(other.mtx_);
    //     data_.swap(other.data_);
    //   }
    //   return *this;
    // }
    void lock() {
      assert(check_for_hierarchy_violation());
      mtx_.lock();
      update_hierarchy_value();
    }
    void unlock() {
      assert(this_thread_hierarchical_value_ == hierarchical_value_);
      this_thread_hierarchical_value_ = previous_hierarchical_value_;
      mtx_.unlock();
    }
  private:
    bool check_for_hierarchy_violation() {
      if (this_thread_hierarchical_value_ <= hierarchical_value_) {
        return false;
      }
      return true;
    }
    void update_hierarchy_value() {
      previous_hierarchical_value_ = this_thread_hierarchical_value_;
      this_thread_hierarchical_value_ = hierarchical_value_;
    }
    std::mutex mtx_;
    const int hierarchical_value_;
    int previous_hierarchical_value_;
    static thread_local int this_thread_hierarchical_value_;
};
thread_local int hierarchical_mutex::this_thread_hierarchical_value_ = INT32_MAX;
void test4() {
  hierarchical_mutex mtx1(1000);
  hierarchical_mutex mtx2(500);
  //上面代码会导致死锁，解决方法，将每一个mutex操作在封在一个函数内这样就不会出现死锁
  std::thread t1([&](){
    mtx1.lock();
    mtx2.lock();
    mtx2.unlock();
    mtx1.unlock();
  });
  t1.join();

  //t2 执行失败， 违反加锁顺序
  // std::thread t2([&](){
  //   mtx2.lock();
  //   mtx1.lock();
  //   mtx1.unlock();
  //   mtx2.unlock();
  // });
  // t2.join();
  
  // t3 执行失败 违反解锁顺序
  // std::thread t3([&](){
  //   mtx1.lock();
  //   mtx2.lock();
  //   mtx1.unlock();
  //   mtx2.unlock();
  // });
  // t3.join();
}

int main() {
  test1();
  test2();
  test3();
  test4();
  return 0;
}