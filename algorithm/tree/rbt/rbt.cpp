#include "rbt.h"
#include <iostream>

using namespace algorithm;

void test0() {
    std::vector<int> nums = {4, 2, 6, 1, 3, 5, 15, 7, 16, 14};
    RBTree<int, int> rbt;
    for (auto num : nums) {
        rbt.insert({num, num});
        if (rbt.checkValid() == false) {
            std::cout << "insert failed: " << num << std::endl;
            exit(1);
        }
    }
    for (auto num : nums) {
        rbt.remove(num);
        if (rbt.checkValid() == false) {
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
    RBTree<int, int> rbt;
    for (auto num : nums) {
        rbt.insert({num, num});
        if (rbt.checkValid() == false) {
            std::cout << "insert failed: " << num << std::endl;
            exit(1);
        }
    }
    for (auto num : nums) {
        rbt.remove(num);
        if (rbt.checkValid() == false) {
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
