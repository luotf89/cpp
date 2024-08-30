/*
代码实现参考
https://www.cnblogs.com/gonghr/p/16060017.html
*/

#include "bst.h"
#include <iostream>


using namespace algorithm;

int main() {
    BSTree<int, int> bst;
    bst.insert({5, 5});
    bst.insert({1, 1});
    bst.insert({2, 2});
    bst.insert({7, 7});
    bst.insert({8, 8});
    bst.insert({4, 4});
    bst.insert({3, 3});
    bst.insert({6, 6});
    bst.insert({9, 9});
    // bst.visualization();
    bst.remove(5);
    bst.visualization();
    auto curr = bst.find(10);
    std::cout << curr << std::endl;
    return 0;
}