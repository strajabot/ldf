#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstring>

#include "../source/arch.hpp"
#include "../source/metadata.hpp"

TEST_CASE("from_integral produces correct decimal strings", "[metadata][sstring]") {
    REQUIRE(strcmp(ldf::sstring::from_integral<0>().data(),                     "0")                    == 0);
    REQUIRE(strcmp(ldf::sstring::from_integral<1>().data(),                     "1")                    == 0);
    REQUIRE(strcmp(ldf::sstring::from_integral<42>().data(),                    "42")                   == 0);
    REQUIRE(strcmp(ldf::sstring::from_integral<100>().data(),                   "100")                  == 0);
    REQUIRE(strcmp(ldf::sstring::from_integral<999>().data(),                   "999")                  == 0);
    REQUIRE(strcmp(ldf::sstring::from_integral<1000>().data(),                  "1000")                 == 0);
    REQUIRE(strcmp(ldf::sstring::from_integral<18446744073709551615ULL>().data(),"18446744073709551615") == 0);
}

TEST_CASE("from_integral is constexpr", "[metadata][sstring]") {
    static_assert(ldf::sstring::from_integral<0>().data()[0]  == '0');
    static_assert(ldf::sstring::from_integral<42>().data()[0] == '4');
    static_assert(ldf::sstring::from_integral<42>().data()[1] == '2');
}

TEST_CASE("Integral compile emits correct assembly directive", "[metadata]") {
    static constexpr ldf::sstring::container section(".s");

    // unsigned
    REQUIRE(strcmp(ldf::meta::Integral<uint64_t{0}>::compile<section>().data(),   ".balign 8\n\t.quad 0\n\t")   == 0);
    REQUIRE(strcmp(ldf::meta::Integral<uint64_t{42}>::compile<section>().data(),  ".balign 8\n\t.quad 42\n\t")  == 0);
    REQUIRE(strcmp(ldf::meta::Integral<uint32_t{7}>::compile<section>().data(),   ".balign 4\n\t.long 7\n\t")   == 0);
    REQUIRE(strcmp(ldf::meta::Integral<uint16_t{3}>::compile<section>().data(),   ".balign 2\n\t.short 3\n\t")  == 0);
    REQUIRE(strcmp(ldf::meta::Integral<uint8_t{255}>::compile<section>().data(),  ".balign 1\n\t.byte 255\n\t") == 0);

    // signed positive
    REQUIRE(strcmp(ldf::meta::Integral<int64_t{100}>::compile<section>().data(),  ".balign 8\n\t.quad 100\n\t") == 0);
    REQUIRE(strcmp(ldf::meta::Integral<int32_t{7}>::compile<section>().data(),    ".balign 4\n\t.long 7\n\t")   == 0);
    REQUIRE(strcmp(ldf::meta::Integral<int16_t{3}>::compile<section>().data(),    ".balign 2\n\t.short 3\n\t")  == 0);
    REQUIRE(strcmp(ldf::meta::Integral<int8_t{127}>::compile<section>().data(),   ".balign 1\n\t.byte 127\n\t") == 0);

    // signed negative
    REQUIRE(strcmp(ldf::meta::Integral<int64_t{-1}>::compile<section>().data(),   ".balign 8\n\t.quad -1\n\t")   == 0);
    REQUIRE(strcmp(ldf::meta::Integral<int32_t{-1}>::compile<section>().data(),   ".balign 4\n\t.long -1\n\t")   == 0);
    REQUIRE(strcmp(ldf::meta::Integral<int16_t{-1}>::compile<section>().data(),   ".balign 2\n\t.short -1\n\t")  == 0);
    REQUIRE(strcmp(ldf::meta::Integral<int8_t{-128}>::compile<section>().data(),  ".balign 1\n\t.byte -128\n\t") == 0);
}

TEST_CASE("Integral compile is constexpr", "[metadata]") {
    static constexpr ldf::sstring::container section(".s");
    // output is ".balign N\n\t.DIRECTIVE ..." — directive letter is at index 12
    static_assert(ldf::meta::Integral<uint64_t{1}>::compile<section>().data()[12] == 'q'); // ".quad"
    static_assert(ldf::meta::Integral<uint32_t{1}>::compile<section>().data()[12] == 'l'); // ".long"
    static_assert(ldf::meta::Integral<uint16_t{1}>::compile<section>().data()[12] == 's'); // ".short"
    static_assert(ldf::meta::Integral<uint8_t{1}>::compile<section>().data()[12]  == 'b'); // ".byte"
}

TEST_CASE("String compile emits correct assembly", "[metadata]") {
    static constexpr ldf::sstring::container section(".my_sec");
    static constexpr ldf::sstring::container hello("hello");

    constexpr auto asm_str = ldf::meta::String<hello>::template compile<section>();

    const char* expected =
        ".pushsection .my_sec.strings, \"MS\", @progbits, 1\n\t"
        "2: .asciz \"hello\"\n\t"
        ".popsection\n\t"
        ".balign 8\n\t"
        ".quad 2b\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("String compile is constexpr", "[metadata]") {
    static constexpr ldf::sstring::container section(".s");
    static constexpr ldf::sstring::container val("x");
    static_assert(
        ldf::meta::String<val>::template compile<section>().data()[1] == 'p'
    ); // ".pushsection"
}

TEST_CASE("Builder compile produces correct full assembly string", "[metadata]") {
    static constexpr ldf::sstring::container section(".my_section");

    using Builder = decltype(
        ldf::meta::Builder<section>{}
            .add(ldf::meta::Integral<uint64_t{3}>{})
            .add(ldf::meta::Integral<uint32_t{7}>{})
    );

    constexpr auto asm_str = Builder::compile();

    const char* expected =
        ".pushsection .my_section, \"M\", @progbits, 8\n\t"
        ".pushsection .my_section.structs, \"M\", @progbits, 16\n\t"
        ".balign 8\n\t"
        "1:\n\t"
        ".balign 8\n\t.quad 3\n\t"
        ".balign 4\n\t.long 7\n\t"
        ".balign 8\n\t"
        ".popsection\n\t"
        ".balign 8\n\t"
        "0: .quad 1b\n\t"
        ".balign 8\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("Builder compile is constexpr", "[metadata]") {
    static constexpr ldf::sstring::container section(".s");
    using B = decltype(ldf::meta::Builder<section>{}
        .add(ldf::meta::Integral<uint8_t{1}>{}));
    static_assert(B::compile().data()[0] == '.');
}

TEST_CASE("Builder with no fields compiles correctly", "[metadata]") {
    static constexpr ldf::sstring::container section(".empty");

    constexpr auto asm_str = ldf::meta::Builder<section>::compile();

    const char* expected =
        ".pushsection .empty, \"M\", @progbits, 8\n\t"
        ".pushsection .empty.structs, \"M\", @progbits, 0\n\t"
        ".balign 1\n\t"
        "1:\n\t"
        ".balign 1\n\t"
        ".popsection\n\t"
        ".balign 8\n\t"
        "0: .quad 1b\n\t"
        ".balign 8\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("Builder with string field compiles correctly", "[metadata]") {
    static constexpr ldf::sstring::container section(".my_section");
    static constexpr ldf::sstring::container greeting("hello");

    using Builder = decltype(
        ldf::meta::Builder<section>{}
            .add(ldf::meta::String<greeting>{})
    );

    constexpr auto asm_str = Builder::compile();

    const char* expected =
        ".pushsection .my_section, \"M\", @progbits, 8\n\t"
        ".pushsection .my_section.structs, \"M\", @progbits, 8\n\t"
        ".balign 8\n\t"
        "1:\n\t"
        ".pushsection .my_section.strings, \"MS\", @progbits, 1\n\t"
        "2: .asciz \"hello\"\n\t"
        ".popsection\n\t"
        ".balign 8\n\t"
        ".quad 2b\n\t"
        ".balign 8\n\t"
        ".popsection\n\t"
        ".balign 8\n\t"
        "0: .quad 1b\n\t"
        ".balign 8\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}

TEST_CASE("Builder with mixed numeric and string fields", "[metadata]") {
    static constexpr ldf::sstring::container section(".ldf");
    static constexpr ldf::sstring::container tag("world");

    using Builder = decltype(
        ldf::meta::Builder<section>{}
            .add(ldf::meta::Integral<uint32_t{5}>{})
            .add(ldf::meta::String<tag>{})
    );

    constexpr auto asm_str = Builder::compile();

    const char* expected =
        ".pushsection .ldf, \"M\", @progbits, 8\n\t"
        ".pushsection .ldf.structs, \"M\", @progbits, 16\n\t"
        ".balign 8\n\t"
        "1:\n\t"
        ".balign 4\n\t.long 5\n\t"
        ".pushsection .ldf.strings, \"MS\", @progbits, 1\n\t"
        "2: .asciz \"world\"\n\t"
        ".popsection\n\t"
        ".balign 8\n\t"
        ".quad 2b\n\t"
        ".balign 8\n\t"
        ".popsection\n\t"
        ".balign 8\n\t"
        "0: .quad 1b\n\t"
        ".balign 8\n\t"
        ".popsection\n\t"
        LOAD_ADDRESS("%0", "0b") "\n\t";

    REQUIRE(strcmp(asm_str.data(), expected) == 0);
}
