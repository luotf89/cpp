#include <cstddef>
#include <future>
#include <iostream>
#include <thread>
#include <vector>


/*
template< class F, class... Args >
std::future<std::invoke_result_t<std::decay_t<F>,
                                 std::decay_t<Args>...>>
    async( std::launch policy, F&& f, Args&&... args );
1、返回值类型：
  返回值为为std::future
  std::future 常用get 和wait 方法
  get(): 阻塞当前线程，直到关联的被调用对象返回，并当代共享结果值设置好，然后设置共享对象的状态为invalid 因此get函数只能调用一次
  wait(): 提供有各种超时版本，阻塞当前线程直到关联的被调用对象返回，不一定等待共享结果的值设置好，不会设置共享对象的状态，可以调用多次
2、std::launch policy可以有以下取值
  1、std::launch::async  采用起一个新线程调度F
  2、std::launch::deferred 延时调度, 等返回值调用wait 或者get方法时 F才被调用
  3、std::launch::async | std::launch::deferred 根据具体的消耗情况来采用是使用延迟调用还是另起线程调用
3、promise 和 future 用于多线程，如果想让promise设置的状态让多个future 共享可以采用share_future
  1、promise设置状态
  2、future获取状态
  3、future获取状态的时候需要 promise还在声明周期内，如果promise已经退出声明周期者future get状态不对
4、packaged_task 是一个类 里面重载了 operator() 将一个带返回值的可调用对象wrapper 一个返回值为void的可调用对象，
  可调用对象的返回值通过get_future来获得，
  packaged_task的拷贝构造函数被删掉了 提供移动构造函数

*/

void test0() {
  std::packaged_task<int(int)> packaged_func([](int a) {return a * a;});
  std::future<int> ret1 = packaged_func.get_future();
  // 传给 async 需要 用std::move(package_func) 原因 package_func是一个左值 传递给 async变成左值引用
  // 后面async采用 decay 类型后的tuple来保存参数，右值无法接受左值引用来传递。
  std::future<void> ret2 = std::async(std::launch::async, std::move(packaged_func), 10);
  std::cout << "ret1: " << ret1.get() << std::endl;
}

void test1() {
  std::promise<int> promise;
  std::shared_future<int> future = promise.get_future();

  std::vector<std::thread> threads;
  threads.emplace_back([](std::promise<int>&& promise) {
    promise.set_value(42);
  }, std::move(promise));
  for (int i = 0; i < 10; i++) {
    threads.emplace_back([](std::shared_future<int> future) {
      std::cout << "===========: " << future.get() << std::endl;
    }, future);
  }
  for (std::size_t i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
}

int main() {
  std::cout << "============== test 0 ============" << std::endl;
  test0();
  std::cout << "============== test 1 ============" << std::endl;
  test1();
}