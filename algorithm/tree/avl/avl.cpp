/*
代码实现参考
https://www.cnblogs.com/gonghr/p/16064797.html
*/

#include "avl.h"
#include <iostream>
#include <string>


using namespace algorithm;

void test0() {
    std::vector<int> nums = {4, 2, 6, 1, 3, 5, 15, 7, 16, 14};
    AVLTree<int, int> avl;
    for (auto num : nums) {
        avl.insert({num, num});
        if (avl.is_balanced() == false) {
            std::cout << "insert failed: " << num << std::endl;
            break;
        }
    }
    std::cout << "================= insert =============\n";
    avl.walk<WalkOrder::INORDER>([](AVLTree<int,int>::NodeTy* node) {
        std::cout << "key:"<< node->key_ << " value: " << node->value_ << std::endl;
    });
    // avl.remove(6);
    // avl.remove(15);
    // avl.remove(16);
    std::cout << "================= remove =============\n";
    // avl.walk<WalkOrder::INORDER>([](AVLTree<int,int>::NodeTy* node) {
    //     std::cout << "key:"<< node->key_ << " value: " << node->value_ << std::endl;
    // });
    // std::cout << "is_balanced: " << avl.is_balanced() << std::endl;
    for (auto num : nums) {
        avl.remove(num);
        if (avl.is_balanced() == false) {
            std::cout << "remove failed: " << num << std::endl;
            break;
        }
    }
    std::cout << "================= print =============\n";
    avl.walk<WalkOrder::INORDER>([](AVLTree<int,int>::NodeTy* node) {
        std::cout << "key:"<< node->key_ << " value: " << node->value_ << std::endl;
    });
}

int main() {
    test0();
    return 0;
}
