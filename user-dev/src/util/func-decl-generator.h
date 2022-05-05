#ifndef _SRC__UTIL__FUNC_DECL_GENERATOR_H_
#define _SRC__UTIL__FUNC_DECL_GENERATOR_H_

#include "util/func-decl-types.h"

#define default_make_decl(name)                                                \
    {                                                                          \
        V_TO_STR(name), {                                                      \
            CAST(void const *, &name)                                          \
        }                                                                      \
    }


#define custom_make_decls(decl_type, decl_name, decl_builder, fwd_decl, ...)   \
    APPLY(fwd_decl, ;, __VA_ARGS__);                                           \
    extern const decl_type decl_name; /* for c++ linkage.  */                  \
    const decl_type        decl_name = { PP_NARG(__VA_ARGS__),                 \
                                         V_TO_STR(decl_name),                  \
                                         { APPLY(decl_builder, COMMA,          \
                                                 __VA_ARGS__) } };

#define make_decls(decl_name, fwd_decl, ...)                                   \
    custom_make_decls(decl_list_t, decl_name, default_make_decl, fwd_decl,     \
                      __VA_ARGS__)

#endif
