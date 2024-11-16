#include <cassert>
#include <type_traits>
#include <iostream>
#include "variant.h"

void test0() {
    algorithm::Variant<int, double, std::string> v;
    auto ret = std::is_same_v<int, algorithm::variant_index_2_type<0, decltype(v)>::type>;
    assert(ret);
    ret = std::is_same_v<double, algorithm::variant_index_2_type<1, decltype(v)>::type>;
    assert(ret);
    ret = std::is_same_v<std::string, algorithm::variant_index_2_type<2, decltype(v)>::type>;
    assert(ret);

    ret = algorithm::variant_type_2_index<int, decltype(v)>::value == 0;
    assert(ret);
    ret = algorithm::variant_type_2_index<double, decltype(v)>::value == 1;
    assert(ret);
    ret = algorithm::variant_type_2_index<std::string, decltype(v)>::value == 2;
    assert(ret);
}

using vector_t = std::vector<int>;

auto& operator<<(auto& out, const vector_t& v)
{
    out << "{ ";
    for (int e : v)
        out << e << ' ';
    return out << '}';
}

void test1() {
        // value-initializes first alternative
    algorithm::Variant<int, std::string> var0;
    assert(var0.index() == -1);
 
    // initializes first alternative with std::string{"STR"};
    algorithm::Variant<std::string, int> var1{std::string{"STR"}};
    assert(var1.index() == 0);
    std::cout << "1) " << var1.get<std::string>() << '\n';
 
    // initializes second alternative with int == 42;
    algorithm::Variant<std::string, int> var2{42};
    assert(var2.has_value<int>());
    std::cout << "2) " << var2.get<int>() << '\n';
 
    // initializes first alternative with std::string{4, 'A'};
    algorithm::Variant<std::string, vector_t, float> var3
    {
         algorithm::in_place_type<std::string>, 4, 'A'
    };
    assert(var3.index() == 0);
    std::cout << "3) " << var3.get<std::string>() << '\n';
 
    // initializes second alternative with std::vector{1,2,3,4,5};
    algorithm::Variant<std::string, vector_t, char> var4
    {
        algorithm::in_place_type<vector_t>, {1, 2, 3, 4, 5}
    };
    assert(var4.index() == 1);
    std::cout << "4) " << var4.get<vector_t>() << '\n';
 
    // initializes first alternative with std::string{"ABCDE", 3};
    algorithm::Variant<std::string, vector_t, bool> var5 {algorithm::in_place_index<0>, "ABCDE", 3};
    assert(var5.index() == 0);
    std::cout << "5) " << var5.get<std::string>() << '\n';
 
    // initializes second alternative with std::vector(4, 42);
    algorithm::Variant<std::string, vector_t, char> var6 {algorithm::in_place_index<1>, 4, 42};
    assert(var6.has_value<1>());
    std::cout << "6) " << var6.get<1>() << '\n';
}

void test2() {
    algorithm::Variant<std::string> v1;
    v1.emplace<0>("abc"); // OK
    std::cout << v1.get<0>() << '\n';
    v1.emplace<std::string>("def"); // OK
    std::cout << v1.get<0>() << '\n';
 
    algorithm::Variant<std::string, std::string> v2;
    v2.emplace<1>("ghi"); // OK
    std::cout << v2.get<1>() << '\n';
}

int main() {
    test0();
    test1();
    test2();
    return 0;
}

