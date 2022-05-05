#include "util/types.h"
#include "test/test-common.h"
#include "util/macro.h"

#define get_result_safe_T(T)                                                   \
    ((sizeof(T) == sizeof(safe_T(T))) &&                                       \
     (GET_IS_INT_BASE(T) == GET_IS_INT_BASE(safe_T(T))) &&                     \
     (GET_IS_FLOAT_BASE(T) == GET_IS_FLOAT_BASE(safe_T(T))) &&                 \
     (GET_IS_PTR(T) == GET_IS_PTR(safe_T(T))) &&                               \
     (TYPE_MAX(T) == TYPE_MAX(safe_T(T))) &&                                   \
     (TYPE_MIN(T) == TYPE_MIN(safe_T(T))) &&                                   \
     (GET_IS_SIGNED(T) == GET_IS_SIGNED(safe_T(T))) &&                         \
     (GET_IS_UNSIGNED(T) == GET_IS_UNSIGNED(safe_T(T))) &&                     \
     (GET_IS_UNSIGNED_INT(T) == GET_IS_UNSIGNED_INT(safe_T(T))) &&             \
     (sizeof_bits(T) == sizeof_bits(safe_T(T))))


#define get_result_generic(T)                                                  \
    ((GET_IS_FLOAT_BASE(T) || GET_IS_INT_BASE(T)) &&                           \
     (GET_IS_UNSIGNED(GET_UNSIGNED_T(T)) || GET_IS_FLOAT_BASE(T)) &&           \
     (GET_IS_SIGNED(GET_SIGNED_T(T))) &&                                       \
     (GET_IS_FLOAT_BASE(safe_T(T)) || GET_IS_INT_BASE(safe_T(T))) &&           \
     (GET_IS_UNSIGNED(GET_UNSIGNED_T(safe_T(T))) ||                            \
      GET_IS_FLOAT_BASE(safe_T(T))) &&                                         \
     (GET_IS_SIGNED(GET_SIGNED_T(safe_T(T)))))


#define get_result_float_invariants(T)                                         \
    (GET_IS_FLOAT_BASE(T) && (!GET_IS_INT_BASE(T)) &&                          \
     (GET_IS_SIGNED(GET_UNSIGNED_T(T))) && (GET_IS_SIGNED(GET_SIGNED_T(T))) && \
     (GET_IS_FLOAT_BASE(GET_UNSIGNED_T(T))) &&                                 \
     (GET_IS_FLOAT_BASE(GET_SIGNED_T(T))))


#define get_result_int_invariants(T)                                           \
    ((!GET_IS_FLOAT_BASE(T)) && (GET_IS_INT_BASE(T)) &&                        \
     (GET_IS_UNSIGNED(GET_UNSIGNED_T(T))) &&                                   \
     (GET_IS_SIGNED(GET_SIGNED_T(T))) &&                                       \
     (GET_IS_INT_BASE(GET_UNSIGNED_T(T))) &&                                   \
     (GET_IS_INT_BASE(GET_SIGNED_T(T))))

#define get_result_T_invariants(T)                                             \
    (((GET_IS_FLOAT_BASE(T) ? get_result_float_invariants(T)                   \
                            : get_result_int_invariants(T)) &&                 \
      (GET_IS_INT_BASE(T) ? get_result_int_invariants(T)                       \
                          : get_result_float_invariants(T))))


#define get_result_ptr_invariants(T)                                           \
    (GET_IS_PTR(ptr_T(T)) &&                                                   \
     (sizeof_deref(DEFAULT_VALUE(ptr_T(T))) == sizeof(T)) &&                   \
     (!GET_IS_INT_BASE(ptr_T(T))) && (!GET_IS_FLOAT_BASE(ptr_T(T))) &&         \
     (sizeof(ptr_T(T)) == sizeof(void *)) &&                                   \
     (is_same_type(I_AS_PTR(DEFAULT_VALUE(ptr_T(T))), ptr_T(T))) &&            \
     (GET_IS_FLOAT_BASE(T) ||                                                  \
      (IS_UNSIGNED(*MAKE_UNSIGNED_PTR(DEFAULT_VALUE(ptr_T(T)))))) &&           \
     (IS_SIGNED(*MAKE_SIGNED_PTR(DEFAULT_VALUE(ptr_T(T))))))


#define check_safe_T(T)           const_assert(get_result_safe_T(T))
#define check_generic(T)          const_assert(get_result_generic(T))
#define check_float_invariants(T) const_assert(get_result_float_invariants(T))
#define check_int_invariants(T)   const_assert(get_result_int_invariants(T))
#define check_T_invariants(T)     const_assert(get_result_T_invariants(T))
#define check_ptr_invariants(T)   const_assert(get_result_ptr_invariants(T))

/* Just check everything compiles and its all const eval. */
APPLY(check_safe_T, ;, ALL_INT_NAMES);
APPLY(check_generic, ;, ALL_TYPE_NAMES);
APPLY(check_float_invariants, ;, ALL_UNIQUE_FLOAT_TYPES);
APPLY(check_int_invariants, ;, ALL_UNIQUE_INT_TYPES);
APPLY(check_T_invariants, ;, ALL_TYPE_NAMES);
APPLY(check_ptr_invariants, ;, ALL_TYPE_NAMES);

int32_t
test_types() {
    test_assert(APPLY(get_result_safe_T, &&, ALL_TYPE_NAMES));
    test_assert(
        __builtin_constant_p(APPLY(get_result_safe_T, &&, ALL_TYPE_NAMES)));

    test_assert(APPLY(get_result_generic, &&, ALL_TYPE_NAMES));

    test_assert(APPLY(get_result_float_invariants, &&, ALL_UNIQUE_FLOAT_TYPES));
    test_assert(APPLY(get_result_int_invariants, &&, ALL_UNIQUE_INT_TYPES));
    test_assert(APPLY(get_result_T_invariants, &&, ALL_TYPE_NAMES));
    test_assert(APPLY(get_result_ptr_invariants, &&, ALL_TYPE_NAMES));

    return 0;
}
