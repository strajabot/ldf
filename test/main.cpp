#include <iostream>

#include "shared.hpp"
#include "sstring.hpp"

int test() {

    const constexpr auto sstring = ldf::sstring::container( "{} {}") ;
    const ldf::metadata* metadata = LDF_CREATE_META(sstring);

    std::cout << "meta_address:" << metadata << '\n';

    return 0;
}

int main() {
    test();

    return 0;
}
