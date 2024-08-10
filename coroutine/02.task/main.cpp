#include <iostream>
#include "task.h"


Task<int> simple_task2() {
  std::cerr << "task 2 start ..." << std::endl;
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);
  std:: cerr << "task 2 returns after 1s." << std::endl;
  co_return 2;
}

Task<int> simple_task3() {
  std:: cerr << "in task 3 start ..." << std::endl;
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
  std:: cerr << "task 3 returns after 2s." << std::endl;
  co_return 3;
}

Task<int> simple_task() {
  std:: cerr << "task start ..." << std::endl;
  auto result2 = co_await simple_task2();
  std:: cerr << "returns from task2: " << result2 << std::endl; ;
  auto result3 = co_await simple_task3();
  std:: cerr << "returns from task3: " << result3 << std::endl;
  co_return 1 + result2 + result3;
}

int main() {
  auto simpleTask = simple_task();
  simpleTask.then([](int result) {
    std::cout<< result << std::endl;
  }).catching([](std::exception &e) {
    std::cerr << "error occurred " << e.what() << std::endl;
  });
  // try {
  //   auto i = simpleTask.get_result();
  //   debug("simple task end from get: ", i);
  // } catch (std::exception &e) {
  //   debug("error: ", e.what());
  // }
  return 0;
}