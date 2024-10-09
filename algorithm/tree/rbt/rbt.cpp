#include "rbt.h"

using namespace algorithm;
int main() {
    RBTree<int, int> rb;
    rb.insert({1, 1});
    rb.insert({2, 2});
    rb.insert({3, 3});
    rb.insert({4, 4});
    rb.insert({5, 5});
    rb.insert({6, 6});
    rb.insert({7, 7});
    rb.insert({8, 8});
    rb.insert({9, 9});
    // rb.visualization();
    rb.remove(1);
    rb.remove(3);
    
    return 0;
}