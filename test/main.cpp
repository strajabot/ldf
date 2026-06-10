#include "../source/metadata.hpp"

#include <cstdint>

static constexpr ldf::sstring::container section(".ldf");
static constexpr ldf::sstring::container plugin_name("my_plugin");

int main() {

    volatile void* ptr1 = ldf::meta::Builder<section>{}
            .add(ldf::meta::Integral<uint8_t{8}>{})
            .add(ldf::meta::Integral<uint16_t{0xFF}>{})
            .add(ldf::meta::String<plugin_name>{})
            .create();


    volatile void* ptr2 = ldf::meta::Builder<section>{}
            .add(ldf::meta::Integral<uint8_t{8}>{})
            .add(ldf::meta::Integral<uint16_t{0xFF}>{})
            .add(ldf::meta::String<plugin_name>{})
            .create();
            
    return 0;

}
