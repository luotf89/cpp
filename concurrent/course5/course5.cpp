#include <mutex>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

/*
  采用静态变量的方式实现单例模式
*/

class A {
  public:
    A():a_(10) {}
  private:
    int a_;
};

template<typename T>
class Singlton0 {
  public:
    static T& getInst() {
      static Singlton0 inst;
      return inst.t;
    }
  private:
    Singlton0() = default;
    Singlton0(const Singlton0&) = delete;
    Singlton0& operator=(const Singlton0&) = delete;
    T t;
};
void test0() {
  const int thread_num = 1024;
  std::vector<std::thread> threads;
  for (int i = 0; i < thread_num; i++) {
    threads.emplace_back([]() {
      std::cout << "thread_id: " << std::this_thread::get_id() << " address: " << &(Singlton0<A>::getInst()) << std::endl;
    });
  }
  for (int i = 0; i < thread_num; i++) {
    threads[i].join();
  }
}

/*
  采用指针的方式 实现单例：
  1、初始化只能初始化一次 可以采用call once函数解决
  2、释放指针需要在最后一个实例被释放的时候进行指针释放，
  可以采用智能指针的方式来解决
*/

template <typename T>
class Singlton1 {
  public:
  // static std::shared_ptr<T> getInst() {
  //   static std::shared_ptr<Singlton1> inst;
  //   static std::once_flag once_flag;
  //   std::call_once(once_flag, [&]() {
  //     inst = std::make_shared<Singlton1>();
  //   });
  //   return inst->instance_;
  // }

  static T* getInst() {
    static std::shared_ptr<Singlton1> inst;
    static std::once_flag once_flag;
    std::call_once(once_flag, [&]() {
      inst = std::shared_ptr<Singlton1>(new Singlton1);
    });
    return inst->instance_;
  }

  ~Singlton1() { 
    delete instance_;
  }
  private:
    Singlton1() {
      // instance_ = std::make_shared<T>();
      instance_ = new T;
    };
    Singlton1(const Singlton1&) = delete;
    Singlton1& operator=(const Singlton1&) = delete;
    std::shared_ptr<T> getElem() {
      return instance_;
    }
    T* instance_;
    // std::shared_ptr<T> instance_;
};

template <typename T>
class Singlton2 {
  public:
  static std::shared_ptr<T> getInst() {
    static std::shared_ptr<Singlton2> inst;
    static std::once_flag once_flag;
    std::call_once(once_flag, [&]() {
      inst = std::shared_ptr<Singlton2>(new Singlton2);
    });
    return inst->instance_;
  }

  ~Singlton2() {}
  private:
    Singlton2() {
      instance_ = std::shared_ptr<T>(new T);
    };
    Singlton2(const Singlton2&) = delete;
    Singlton2& operator=(const Singlton2&) = delete;
    std::shared_ptr<T> instance_;
};

void test1() {
  const int thread_num = 1024;
  std::vector<std::thread> threads1;
  for (int i = 0; i < thread_num; i++) {
    threads1.emplace_back([]() {
      std::cout << "thread_id: " << std::this_thread::get_id() << " address: " << Singlton1<A>::getInst() << std::endl;
    });
  }
  for (int i = 0; i < thread_num; i++) {
    threads1[i].join();
  }

  std::vector<std::thread> threads2;
  for (int i = 0; i < thread_num; i++) {
    threads2.emplace_back([]() {
      std::cout << "thread_id: " << std::this_thread::get_id() << " address: " << Singlton2<A>::getInst().get() << std::endl;
    });
  }
  for (int i = 0; i < thread_num; i++) {
    threads2[i].join();
  }
}


int main() {
  // std::cout << "============== test 0 ============" << std::endl;
  // test0();
  std::cout << "============== test 1 ============" << std::endl;
  test1();
}