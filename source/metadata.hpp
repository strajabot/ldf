#ifndef LDF_METADATA
#define LDF_METADATA

#include "arch.hpp"
#include "sstring.hpp"

#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace ldf::meta
{

template <size_t align>
constexpr auto pad_directive() {
    return ldf::sstring::concat(".balign ", ldf::sstring::from_integral<align>(), "\n\t");
}

template <typename T>
constexpr auto value_directive() {
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
        "ldf: meta::directive: unsupported type size");

    if constexpr (sizeof(T) == 1)      return ldf::sstring::concat(".byte ");
    else if constexpr (sizeof(T) == 2) return ldf::sstring::concat(".short ");
    else if constexpr (sizeof(T) == 4) return ldf::sstring::concat(".long ");
    else if constexpr (sizeof(T) == 8) return ldf::sstring::concat(".quad ");
}

template<auto V>
struct Integral {

    static constexpr auto size() {
        return sizeof(decltype(V));
    }

    static constexpr auto align() {
        return alignof(decltype(V));
    }

    template<const auto& /*Section*/>
    static constexpr auto compile() {
        using T = decltype(V);
        static_assert(std::is_integral_v<T>,
            "ldf: meta::Integral: only supports integral types");

        constexpr auto string = ldf::sstring::from_integral<V>();
        return ldf::sstring::concat(
            pad_directive<alignof(T)>(),
            value_directive<T>(), string, "\n\t"
        );
    }

};

template<const auto& Value>
struct String {

    static constexpr auto size() {
        return sizeof(uintptr_t);
    }

    static constexpr auto align() {
        return alignof(uintptr_t);
    }

    template<const auto& Section>
    static constexpr auto compile() {
        // todo: technically, entsize == 1 can be a wrong assumption
        return ldf::sstring::concat(
            ".pushsection ", ldf::sstring::concat(Section, ".strings"), ", \"MS\", @progbits, 1\n\t",
            "2: .asciz \"", ldf::sstring::escape<Value>(), "\"\n\t",
            ".popsection\n\t",
            pad_directive<alignof(uintptr_t)>(),
            value_directive<uintptr_t>(), "2b", "\n\t"
        );
    }

};


template<const auto& Section, typename... Fields>
class Builder {

    static constexpr bool check_section() {
        if (Section.size() < 2)
            return false;

        if (Section.value[0] != '.')
            return false;

        for (size_t i = 1; i < Section.size(); ++i) {
            const char c = Section.value[i];
            const bool first = i == 1;

            if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && (first || c < '0' || c > '9') && c != '_')
                return false;
        }
        return true;
    }

    static_assert(check_section(), "ldf: meta::Builder section name must be \"\\.[a-zA-Z_][a-zA-Z0-9_]*\"");

    template<typename First, typename... Rest>
    static constexpr size_t size_impl(size_t offset) {
        constexpr size_t field_align = First::align();
        offset = (offset + field_align - 1) & ~(field_align - 1);
        offset += First::size();
        if constexpr (sizeof...(Rest) == 0)
            return offset;
        else
            return size_impl<Rest...>(offset);
    }

    static constexpr size_t size() {
        if constexpr (sizeof...(Fields) == 0)
            return 0;
        else {
            constexpr size_t raw = size_impl<Fields...>(0);
            constexpr size_t a = align();
            return (raw + a - 1) & ~(a - 1);
        }
    }

    static constexpr size_t align() {
        if constexpr (sizeof...(Fields) == 0)
            return 1;
        else
            return std::max({Fields::align()...});
    }


    template<typename First, typename... Rest>
    static constexpr auto concat_fields_impl() {
        if constexpr (sizeof...(Rest) == 0)
            return First::template compile<Section>();
        else
            return ldf::sstring::concat(
                First::template compile<Section>(),
                concat_fields_impl<Rest...>()
            );
    }

    static constexpr auto compile_fields() {
        if constexpr (sizeof...(Fields) == 0)
            return ldf::sstring::container<1>{};
        else
            return concat_fields_impl<Fields...>();
    }

    static constexpr auto compile_struct() {
        constexpr auto entsize = ldf::sstring::from_integral<size()>();
        // todo: by passing size as octets we are assuming CHAR_BIT == 8
        return ldf::sstring::concat(
            ".pushsection ", ldf::sstring::concat(Section, ".structs"), ", \"M\", @progbits, ", entsize, "\n\t",
            pad_directive<align()>(),
            "1:\n\t",
            compile_fields(),
            pad_directive<align()>(), // might be necessary if gcc requires the size of section to be multiple of entsize (last pad)
            ".popsection\n\t"
        );
    }

public:
    template<auto Value>
    constexpr auto add(Integral<Value> /*f*/) const {
        return Builder<Section, Fields..., Integral<Value>>{};
    }

    template<const auto& Value>
    constexpr auto add(String<Value> /*f*/) const {
        return Builder<Section, Fields..., String<Value>>{};
    }

    static constexpr auto compile() {
        constexpr auto entsize = ldf::sstring::from_integral<sizeof(uintptr_t)>();
        // todo: by passing size as octets we are assuming CHAR_BIT == 8
        return ldf::sstring::concat(
            ".pushsection ", Section, ", \"M\", @progbits, ", entsize ,"\n\t",
            compile_struct(),
            pad_directive<alignof(uintptr_t)>(),
            "0: ", value_directive<uintptr_t>(), "1b", "\n\t",
            pad_directive<alignof(uintptr_t)>(),
            ".popsection\n\t",
            LOAD_ADDRESS("%0", "0b"), "\n\t"
        );
    }

    void* create() const {
        void* meta;
        asm volatile((compile()) : "=r"(meta) : : "memory");
        return meta;
    }
};

} // namespace ldf

#endif
