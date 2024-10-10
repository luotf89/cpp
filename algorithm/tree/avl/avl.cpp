/*
代码实现参考
https://www.cnblogs.com/gonghr/p/16064797.html
*/

#include "avl.h"
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>


using namespace algorithm;

void test0() {
    std::vector<int> nums = {4, 2, 6, 1, 3, 5, 15, 7, 16, 14};
    AVLTree<int, int> avl;
    for (auto num : nums) {
        avl.insert({num, num});
        if (avl.checkValid() == false) {
            std::cout << "insert failed: " << num << std::endl;
            exit(1);
        }
    }
    for (auto num : nums) {
        avl.remove(num);
        if (avl.checkValid() == false) {
            std::cout << "remove failed: " << num << std::endl;
            exit(1);
        }
    }
}

void test1() {
    srand((unsigned int)time(0));
    constexpr size_t N = 10000;
    std::vector<int> nums(N);
    for (size_t i = 0; i < N; ++i) {
        nums[i] = rand() % 1000000;
    }
    AVLTree<int, int> avl;
    for (auto num : nums) {
        avl.insert({num, num});
        if (avl.checkValid() == false) {
            std::cout << "insert failed: " << num << std::endl;
            exit(1);
        }
    }
    for (auto num : nums) {
        avl.remove(num);
        if (avl.checkValid() == false) {
            std::cout << "remove failed: " << num << std::endl;
            exit(1);
        }
    }
}

int main() {
    test0();
    test1();
    return 0;
}
