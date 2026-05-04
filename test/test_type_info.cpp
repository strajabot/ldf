#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstring>

#include "../proposal/include/ldf/type_info.hpp"

TEST_CASE("type_name strings are correct", "[type_info]") {
    REQUIRE(strcmp(ldf::type_name<uint8_t>::value.data(),  "u8")  == 0);
    REQUIRE(strcmp(ldf::type_name<uint16_t>::value.data(), "u16") == 0);
    REQUIRE(strcmp(ldf::type_name<uint32_t>::value.data(), "u32") == 0);
    REQUIRE(strcmp(ldf::type_name<uint64_t>::value.data(), "u64") == 0);
    REQUIRE(strcmp(ldf::type_name<int8_t>::value.data(),   "i8")  == 0);
    REQUIRE(strcmp(ldf::type_name<int16_t>::value.data(),  "i16") == 0);
    REQUIRE(strcmp(ldf::type_name<int32_t>::value.data(),  "i32") == 0);
    REQUIRE(strcmp(ldf::type_name<int64_t>::value.data(),  "i64") == 0);
    REQUIRE(strcmp(ldf::type_name<float>::value.data(),    "f32") == 0);
    REQUIRE(strcmp(ldf::type_name<double>::value.data(),   "f64") == 0);
}

TEST_CASE("type_name values are constexpr", "[type_info]") {
    static_assert(ldf::type_name<uint32_t>::value.data()[0] == 'u');
    static_assert(ldf::type_name<int32_t>::value.data()[0]  == 'i');
    static_assert(ldf::type_name<float>::value.data()[0]    == 'f');
}
