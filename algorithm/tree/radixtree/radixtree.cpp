#include "radixtree.h"
#include <cassert>
#include <random>
#include <string>

using namespace algorithm;

std::pair<std::string, int> generator_prefix(unsigned long seed) {
    std::random_device rd;
    std::mt19937 generator(seed + rd());
    // std::mt19937 generator(seed);
    std::uniform_int_distribution<int> distribution0(1, 100);
    std::uniform_int_distribution<int> distribution1(0, 25);
    // std::uniform_int_distribution<int> distribution1(0, 5);
    int len = distribution0(generator);
    std::string prefix;
    for(int i = 0; i < len; i++) {
        prefix += 'a' +  distribution1(generator);
    }
    return {prefix, len};
}

void test() {
    RadixTree<int> radixtree;
    std::set<std::string> prefixes;
    int num = 100000;
    unsigned long seed = 0;
    for (int i = 0; i < num;i++) {
        auto prefix = generator_prefix(seed+i);
        prefixes.insert(prefix.first);
        radixtree.insert(prefix.first, prefix.second);
        assert(radixtree.isValid());
    }
    for (auto prefix: prefixes) {
        assert(radixtree.search(prefix));
        radixtree.remove(prefix);
        assert(radixtree.isValid());
    }
    radixtree.visualization("graph1.dot");
}

int main() {
    test();
    return 0;
}