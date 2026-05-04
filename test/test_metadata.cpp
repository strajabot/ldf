#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstring>

#include "../source/arch.hpp"
#include "../source/metadata.hpp"

TEST_CASE("from_uint produces correct decimal strings", "[metadata][sstring]") {
    REQUIRE(strcmp(ldf::sstring::from_uint<0>().data(),                     "0")                    == 0);
    REQUIRE(strcmp(ldf::sstring::from_uint<1>().data(),                     "1")                    == 0);
    REQUIRE(strcmp(ldf::sstring::from_uint<42>().data(),                    "42")                   == 0);
    REQUIRE(strcmp(ldf::sstring::from_uint<100>().data(),                   "100")                  == 0);
    REQUIRE(strcmp(ldf::sstring::from_uint<999>().data(),                   "999")                  == 0);
    REQUIRE(strcmp(ldf::sstring::from_uint<1000>().data(),                  "1000")                 == 0);
    REQUIRE(strcmp(ldf::sstring::from_uint<18446744073709551615ULL>().data(),"18446744073709551615") == 0);
}

TEST_CASE("from_uint is constexpr", "[metadata][sstring]") {
    static_assert(ldf::sstring::from_uint<0>().data()[0]  == '0');
    static_assert(ldf::sstring::from_uint<42>().data()[0] == '4');
    static_assert(ldf::sstring::from_uint<42>().data()[1] == '2');
}

TEST_CASE("IntegralField compile emits correct assembly directive", "[metadata]") {
    static constexpr ldf::sstring::container section("s");
    REQUIRE(strcmp(ldf::IntegralField<uint64_t{0}>::compile<section>().data(),   ".quad 0\n\t")   == 0);
    REQUIRE(strcmp(ldf::IntegralField<uint64_t{42}>::compile<section>().data(),  ".quad 42\n\t")  == 0);
    REQUIRE(strcmp(ldf::IntegralField<uint32_t{7}>::compile<section>().data(),   ".long 7\n\t")   == 0);
    REQUIRE(strcmp(ldf::IntegralField<uint16_t{3}>::compile<section>().data(),   ".short 3\n\t")  == 0);
    REQUIRE(strcmp(ldf::IntegralField<uint8_t{255}>::compile<section>().data(),  ".byte 255\n\t") == 0);
}

TEST_CASE("IntegralField compile is constexpr", "[metadata]") {
    static constexpr ldf::sstring::container section("s");
    static_assert(ldf::IntegralField<uint64_t{1}>::compile<section>().data()[1] == 'q'); // ".quad"
    static_assert(ldf::IntegralField<uint32_t{1}>::compile<section>().data()[1] == 'l'); // ".long"
    static_assert(ldf::IntegralField<uint16_t{1}>::compile<section>().data()[1] == 's'); // ".short"
    static_assert(ldf::IntegralField<uint8_t{1}>::compile<section>().data()[1]  == 'b'); // ".byte"
}

TEST_CASE("StringField compile emits correct assembly", "[metadata]") {
    static constexpr ldf::sstring::container section("my.sec");
    static constexpr ldf::sstring::container hello("hello");

    constexpr auto asm_str = ldf::StringField<hello>::template compile<section>();

    const char* expected =
        ".pushsection my.sec.strings, \"\", @progbits\n\t"
        "2: .asciz \"hello\"\n\t"
        ".popsection\n\t"
        ".quad 2b\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("StringField compile is constexpr", "[metadata]") {
    static constexpr ldf::sstring::container section("s");
    static constexpr ldf::sstring::container val("x");
    static_assert(
        ldf::StringField<val>::template compile<section>().data()[1] == 'p'
    ); // ".pushsection"
}

TEST_CASE("MetadataBuilder compile produces correct full assembly string", "[metadata]") {
    static constexpr ldf::sstring::container section("my.section");

    using Builder = decltype(
        ldf::MetadataBuilder<section>{}
            .add(ldf::IntegralField<uint64_t{3}>{})
            .add(ldf::IntegralField<uint32_t{7}>{})
    );

    constexpr auto asm_str = Builder::compile();

    const char* expected =
        ".pushsection my.section, \"\", @progbits\n\t"
        "0: .quad 1f\n\t"
        ".popsection\n\t"
        ".pushsection my.section.structs, \"\", @progbits\n\t"
        "1:\n\t"
        ".quad 3\n\t"
        ".long 7\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("MetadataBuilder compile is constexpr", "[metadata]") {
    static constexpr ldf::sstring::container section("s");
    using B = decltype(ldf::MetadataBuilder<section>{}
        .add(ldf::IntegralField<uint8_t{1}>{}));
    static_assert(B::compile().data()[0] == '.');
}

TEST_CASE("MetadataBuilder with no fields compiles correctly", "[metadata]") {
    static constexpr ldf::sstring::container section("empty");

    constexpr auto asm_str = ldf::MetadataBuilder<section>::compile();

    const char* expected =
        ".pushsection empty, \"\", @progbits\n\t"
        "0: .quad 1f\n\t"
        ".popsection\n\t"
        ".pushsection empty.structs, \"\", @progbits\n\t"
        "1:\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("MetadataBuilder with string field compiles correctly", "[metadata]") {
    static constexpr ldf::sstring::container section("my.section");
    static constexpr ldf::sstring::container greeting("hello");

    using Builder = decltype(
        ldf::MetadataBuilder<section>{}
            .add(ldf::StringField<greeting>{})
    );

    constexpr auto asm_str = Builder::compile();

    const char* expected =
        ".pushsection my.section, \"\", @progbits\n\t"
        "0: .quad 1f\n\t"
        ".popsection\n\t"
        ".pushsection my.section.structs, \"\", @progbits\n\t"
        "1:\n\t"
        ".pushsection my.section.strings, \"\", @progbits\n\t"
        "2: .asciz \"hello\"\n\t"
        ".popsection\n\t"
        ".quad 2b\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("MetadataBuilder with mixed numeric and string fields", "[metadata]") {
    static constexpr ldf::sstring::container section("data");
    static constexpr ldf::sstring::container tag("world");

    using Builder = decltype(
        ldf::MetadataBuilder<section>{}
            .add(ldf::IntegralField<uint32_t{5}>{})
            .add(ldf::StringField<tag>{})
    );

    constexpr auto asm_str = Builder::compile();

    const char* expected =
        ".pushsection data, \"\", @progbits\n\t"
        "0: .quad 1f\n\t"
        ".popsection\n\t"
        ".pushsection data.structs, \"\", @progbits\n\t"
        "1:\n\t"
        ".long 5\n\t"
        ".pushsection data.strings, \"\", @progbits\n\t"
        "2: .asciz \"world\"\n\t"
        ".popsection\n\t"
        ".quad 2b\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}
