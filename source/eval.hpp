#ifndef LDF_EVAL
#define LDF_EVAL

#include <string_view>
#include "sstring.hpp"


namespace ldf::detail
{
    template <typename Arg>
    size_t eval_size(std::string_view options, Arg arg) {

    }

    // integral
    template <typename Integral>
    size_t eval_size(std::string_view format_spec, Integral integral)
    {
    
        format_spec.size()
    }

} // ldf::detail

template <typename Arg>


template <typename Arg>
ldf::sstring::container<> eval(ldf::sstring format, size_t idx, Arg arg)
{

}



#endif