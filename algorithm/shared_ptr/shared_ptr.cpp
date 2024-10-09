
#include <iostream>
#include "shared_ptr.h"

class A: public algorithm::EnableSharedFromThis<A> {
    int a_;
public:
    A() {
        std::cout << "construct A\n";
    }

    void func() {
        std::cout << "address:" << shared_form_this().get() << std::endl;
    }

    ~A() {
        std::cout << "deconstruct A\n";
    }
};

class B:public A {
public:
    B() {
        std::cout << "construct B\n";
    }
    ~B() {
        std::cout << "deconstruct B\n";
    }
};


using namespace algorithm;

int main() {
    SharedPtr<A> a(new A());
    std::cout << a.use_count() << "\n";
    SharedPtr<A> b = a.get()->shared_form_this();
    std::cout << a.use_count() << "\n";
    SharedPtr<A> c = std::move(b);
    std::cout << c.use_count() << "\n";

    SharedPtr<A> d(new  B(), [](B* ptr){ delete ptr;});
    std::cout << d.use_count() << "\n";
    SharedPtr<A> e = d;
    std::cout << e.use_count() << "\n";
    SharedPtr<A> f = std::move(e);
    std::cout << f.use_count() << "\n";

}