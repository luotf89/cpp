#include <iostream>
#include "functional.h"

int main() {
    algorithm::Function<void()> func1 = []{
        std::cout << "func1" << std::endl;
    };
    func1();
    return 0;
}