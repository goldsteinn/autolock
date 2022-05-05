#include <stdint.h>

#include "util/func-decl-generator.h"
#include "util/macro.h"

#include "test/test-decls.h"

#define decl_test(name) extern int32_t name();

make_decls(tests, decl_test, TESTS);
