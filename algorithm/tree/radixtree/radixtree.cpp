#include "radixtree.h"
#include <cassert>
#include <random>
#include <vector>

using namespace algorithm;

std::pair<std::string, int> generator_prefix(unsigned long seed) {
    std::random_device rd;
    std::mt19937 generator(seed + rd());
    std::uniform_int_distribution<int> distribution0(1, 1000);
    std::uniform_int_distribution<int> distribution1(0, 25);
    int len = distribution0(generator);
    std::string prefix;
    for(int i = 0; i < len; i++) {
        prefix += 'a' +  distribution1(generator);
    }
    return {prefix, len};
}

void test() {
    RadixTree<int> radixtree;
    std::vector<std::string> prefixes;
    int num = 10000;
    unsigned long seed = 0;
    for (int i = 0; i < num;i++) {
        auto prefix = generator_prefix(seed+i);
        prefixes.emplace_back(prefix.first);
        radixtree.insert(prefix.first, prefix.second);
        assert(radixtree.isValid());  
    }
    // radixtree.visualization("graph.dot");
    for (int i = 0; i < num; i++) {
        assert(radixtree.search(prefixes[i]));
        // radixtree.remove(prefixes[i]);
        // assert(radixtree.isValid());
    }
}

int main() {
    test();
    return 0;
}