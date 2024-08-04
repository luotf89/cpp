#include <thread>
#include <iostream>

/*
仿函数做作为线程函数
*/
void test1() {
  class func{
    public:
    void operator()(std::string str) {
      std::cout << str << std::endl;
    }
    int a;
  };
  // 创建匿名对象 下面方法编译不过
  // std::thread t1(func(), "test1");

  // 可以采用下面方法
  std::thread t1((func()), "test1");

  // 或者不采用匿名对象
  // func b;
  // std::thread t1(b, "test1");
  t1.join();
}

/*
成员函数做作为线程函数
*/
void test2() {
  class A{
    public:
    void print(std::string str) {
      std::cout << str << std::endl;
    }
    int a;
  };

  // 成员函数前面一定要加 "&" 而普通函数前面可加可不加 "&"
  A a;
  std::thread t1(&A::print, &a, "test2");

  t1.join();
}

int main() {
  test1();
  test2();
  return 0;
}