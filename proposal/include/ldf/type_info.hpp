#ifndef LDF_TYPE_INFO
#define LDF_TYPE_INFO

#include <cstdint>
#include "../../../source/sstring.hpp"

namespace ldf {

// Primary template — intentionally undefined so unsupported types cause a clear linker/compile error.
template<typename T>
struct type_name;

template<> struct type_name<uint8_t>  { static constexpr auto value = sstring::container("u8");  };
template<> struct type_name<uint16_t> { static constexpr auto value = sstring::container("u16"); };
template<> struct type_name<uint32_t> { static constexpr auto value = sstring::container("u32"); };
template<> struct type_name<uint64_t> { static constexpr auto value = sstring::container("u64"); };
template<> struct type_name<int8_t>   { static constexpr auto value = sstring::container("i8");  };
template<> struct type_name<int16_t>  { static constexpr auto value = sstring::container("i16"); };
template<> struct type_name<int32_t>  { static constexpr auto value = sstring::container("i32"); };
template<> struct type_name<int64_t>  { static constexpr auto value = sstring::container("i64"); };
template<> struct type_name<float>    { static constexpr auto value = sstring::container("f32"); };
template<> struct type_name<double>   { static constexpr auto value = sstring::container("f64"); };

} // namespace ldf

#endif
