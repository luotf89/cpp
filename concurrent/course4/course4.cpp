#include <mutex>
#include <iostream>
#include <map>
#include <shared_mutex>
#include <thread>

/*
  采用延迟加锁的方式传递 mtx 者认为 unique_lock没有拥有锁，
  只有后续调用 lock 之后才认为unique_lock拥有锁
*/
void test_owner0() {
  std::mutex mtx;
  std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
  // 输出为 0
  std::cout << "owner lock: " << lock.owns_lock() << std::endl;
  mtx.lock();
  // 输出为 0
  std::cout << "owner lock: " << lock.owns_lock() << std::endl;
  mtx.unlock();
  lock.lock();
  // 输出为 1
  std::cout << "owner lock: " << lock.owns_lock() << std::endl;
}

/*
  采用 领养锁 无论mtx是否加锁，都认为 改lock已经 拥有锁，
  领养锁 一般是利用 unique_lock 的释放机制
  因此将加锁的mtx 采用领养的方式传递给unique
  对于未加锁的mtx，如果采用领养的方式传递给unique_lock
  者unique_lock 无法对mtx加锁，
*/
void test_owner1() {
  std::mutex mtx;
  std::unique_lock<std::mutex> lock(mtx, std::adopt_lock);
  // 输出为 1
  std::cout << "owner lock: " << lock.owns_lock() << std::endl;
  mtx.lock();
  // 输出为 1
  std::cout << "owner lock: " << lock.owns_lock() << std::endl;
  mtx.unlock();
  // 输出为 1
  std::cout << "owner lock: " << lock.owns_lock() << std::endl;
  // lock.lock() 报错只有没有owner 的lock才可以加锁，领养的方式者认为lock已经拥有锁
}

void test0() {
  test_owner0();
  test_owner1();
}


class A {
  public:
    A() = default;
    int read(int key) const {
      std::shared_lock<std::shared_mutex> shared_lock(shared_mtx_);
      if (datas_.count((key))) {
        int value = datas_.at(key);
        return value;
      }
      return 0;
    }
    
    void write(std::pair<int, int> elem) {
      std::lock_guard<std::shared_mutex> lock(shared_mtx_);
      datas_.insert(elem);
    }
  private:
    std::map<int, int> datas_;
    mutable std::shared_mutex shared_mtx_;
};

void test1() {
  A a;
  std::thread t1([&] () {
    for (int i = 0; i < 10000; i++) {
      a.write({i, i+1});
    }
  });

  std::thread t2([&] () {
    for (int i = 0; i < 100; i++) {
      for (int j = 0; j < 100; j++) {
        std::cout << "thread 2: " << a.read(j*100+i) << std::endl;
      }
    }
  });

  std::thread t3([&] () {
    for (int i = 0; i < 100; i++) {
      for (int j = 0; j < 100; j++) {
        std::cout << "thread 3: " << a.read(i*100+j) << std::endl;
      }
    }
  });

  t1.join();
  t2.join();
  t3.join();
}

int main() {
  std::cout << "============== test 0 ============" << std::endl;
  test0();
  std::cout << "============== test 1 ============" << std::endl;
  test1();
}