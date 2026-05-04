#ifndef LDF_SHARED
#define LDF_SHARED

#include "sstring.hpp"


#define LDF_META_STRINGS ".meta.ldf.strings"
#define LDF_META_FORMATS ".meta.ldf.formats"

#define LDF_META_ATTRS(name) __attribute__((section(name)), used)


#define LDF_FILE __FILE__
#define LDF_LINE __LINE__

namespace ldf {

struct metadata {
    const char* fstring;
    const char* file;
    int line;
};

}

/**
 * @brief allocates metadata needed for the host to parse the formats.
 */
#define LDF_CREATE_META(format)                                                                         \
    [&]() {                                                                                             \
        static const LDF_META_ATTRS(LDF_META_STRINGS) ldf::sstring::container meta_fstring(format);     \
        static const LDF_META_ATTRS(LDF_META_STRINGS) ldf::sstring::container meta_file(LDF_FILE) ;     \
        static const LDF_META_ATTRS(LDF_META_FORMATS) ldf::metadata meta = {                            \
            .fstring = meta_fstring.c_str(),                                                            \
            .file = meta_file.c_str(),                                                                  \
            .line = LDF_LINE                                                                            \
        };                                                                                              \
        return &meta;                                                                                   \
    }()



#endif

