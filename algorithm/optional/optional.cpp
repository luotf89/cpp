#include <iostream>
#include "optional.h"


void test0() {
    algorithm::Optional<int> opt;
    if (opt) {
        std::cout << "opt has value " << *opt << std::endl;
    } else {
        std::cout << "opt has no value" << std::endl;
    }

    opt = 42;
    if (opt) {
        std::cout << "opt has value " << *opt << std::endl;
    } else {
        std::cout << "opt has no value" << std::endl;
    }

    opt = {};
    if (opt) {
        std::cout << "opt has value " << *opt << std::endl;
    } else {
        std::cout << "opt has no value" << std::endl;
    }
    opt = 3;
    opt.reset();
    if (opt) {
        std::cout << "opt has value " << *opt << std::endl;
    } else {
        std::cout << "opt has no value" << std::endl;
    }
    opt = 3;
    algorithm::Optional<double> opt2;
    opt2 = opt;
    if (opt2) {
        std::cout << "opt2 has value " << *opt2 << std::endl;
    } else {
        std::cout << "opt2 has no value" << std::endl;
    }

    algorithm::Optional<double> opt3 = opt;
    if (opt3) {
        std::cout << "opt3 has value " << *opt3 << std::endl;
    } else {
        std::cout << "opt3 has no value" << std::endl;
    }

    algorithm::Optional<int> opt4(algorithm::in_place, 42);
    if (opt4) {
        std::cout << "opt4 has value " << *opt4 << std::endl;
    }else {
        std::cout << "opt4 has no value" << std::endl;
    }
}

void test1() {
    algorithm::Optional<int> opt(42);
    std::cout << "opt has value " << opt.value_or(100) << std::endl;

    opt.reset();
    std::cout << "opt2 has value " << opt.value_or(100) << std::endl;

}



int main() {
    test0();
    test1();
    return 0;
}