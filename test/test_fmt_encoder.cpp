#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstring>

#include "../proposal/include/ldf/fmt_encoder.hpp"

// Helper: create a FmtProvider type from a string literal via a local struct in a macro.
#define MAKE_FMT(name, str) \
    struct name { constexpr auto operator()() const { return ldf::sstring::container(str); } }

TEST_CASE("auto-indexed placeholder gets type and sorted index injected", "[fmt_encoder]") {
    // Single arg: {} -> {0:u32:}
    MAKE_FMT(Fmt1, "{}");
    using Enc = ldf::FMTStringEncoder<Fmt1, uint32_t>;
    REQUIRE(strcmp(Enc::value.data(), "{0:u32:}") == 0);
}

TEST_CASE("multiple auto-indexed placeholders with reordering", "[fmt_encoder]") {
    // Args: uint8_t (align 1), uint64_t (align 8)
    // original_to_sorted: uint8_t->1, uint64_t->0
    // "{} {}" -> "{1:u8:} {0:u64:}"
    MAKE_FMT(Fmt2, "{} {}");
    using Enc = ldf::FMTStringEncoder<Fmt2, uint8_t, uint64_t>;
    REQUIRE(strcmp(Enc::value.data(), "{1:u8:} {0:u64:}") == 0);
}

TEST_CASE("explicit index with format spec", "[fmt_encoder]") {
    // "{1:x}" with args (uint8_t, uint32_t)
    // original_to_sorted: uint8_t->1, uint32_t->0
    // "{1:x}" -> "{0:u32:x}"
    MAKE_FMT(Fmt3, "{1:x}");
    using Enc = ldf::FMTStringEncoder<Fmt3, uint8_t, uint32_t>;
    REQUIRE(strcmp(Enc::value.data(), "{0:u32:x}") == 0);
}

TEST_CASE("passthrough characters are preserved", "[fmt_encoder]") {
    MAKE_FMT(Fmt4, "val={}!");
    using Enc = ldf::FMTStringEncoder<Fmt4, uint32_t>;
    REQUIRE(strcmp(Enc::value.data(), "val={0:u32:}!") == 0);
}

TEST_CASE("encoded value is a constexpr", "[fmt_encoder]") {
    MAKE_FMT(Fmt5, "{}");
    using Enc = ldf::FMTStringEncoder<Fmt5, uint32_t>;
    static_assert(Enc::value.data()[0] == '{');
}
