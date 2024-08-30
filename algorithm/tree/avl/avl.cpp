/*
代码实现参考
https://www.cnblogs.com/gonghr/p/16064797.html
*/

#include "avl.h"
#include <iostream>


using namespace algorithm;

int main() {
    AVLTree<int, int> avl;
    avl.insert({1, 1});
    avl.insert({2, 2});
    avl.insert({3, 3});
    avl.insert({4, 4});
    avl.insert({5, 5});
    avl.insert({6, 6});
    avl.insert({7, 7});
    avl.insert({8, 8});
    avl.insert({9, 9});
    // avl.visualization();
    avl.remove(1);
    avl.remove(3);
    avl.visualization();
    auto curr = avl.find(10);
    std::cout << curr << std::endl;
    return 0;
}