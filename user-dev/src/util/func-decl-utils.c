#include "util/func-decl-utils.h"

#include "util/error-util.h"
#include "util/memory-util.h"
#include "util/regex-util.h"

#include <stdint.h>

/* TODO: replace regex logic in fnmatch. */
enum { decl_do_nothing = 0, decl_re_built = 2, decl_ran = 4 };
enum { decl_did_one = 1, decl_did_any = 0 };


static uint32_t
build_re_wildcard_matcher(regex_t * restrict re,
                          char const * restrict decl_to_run) {
    char const * decl_to_run_next;
    uint64_t     copy_len;
    uint32_t     match_any;
    /* Max possible length. */
    char   pattern_buf[strlen_c(decl_to_run) * 2 + 3];
    char * pattern_buf_pos;
    die_assert(decl_to_run);
    die_assert(re);

    pattern_buf[0]  = '^';
    pattern_buf_pos = pattern_buf + 1;


    match_any = decl_did_one;
    for (;;) {
        decl_to_run_next = strchrnul_c(decl_to_run, '*');
        die_assert(decl_to_run_next >= decl_to_run);
        copy_len = (decl_to_run_next - decl_to_run);
        if (!(*decl_to_run_next)) {
            memcpy_c(pattern_buf_pos, decl_to_run, copy_len);
            pattern_buf_pos[copy_len]     = '$';
            pattern_buf_pos[copy_len + 1] = '\0';
            break;
        }

        match_any = decl_did_any;
        memcpy_c(pattern_buf_pos, decl_to_run, copy_len);
        pattern_buf_pos[copy_len]     = '.';
        pattern_buf_pos[copy_len + 1] = '*';
        pattern_buf_pos += (copy_len + 2);
        decl_to_run = decl_to_run_next + 1;
    }
    safe_re_compile(re, pattern_buf, REG_ICASE | REG_NOSUB);
    return match_any;
}

static int32_t
is_match(regex_t const * restrict re, char const * restrict decl_name) {
    die_assert(re, "NULL re (should be impossible)!");
    die_assert(decl_name, "NULL decl_name, issue with test struct generator!");

    return !safe_re_exec(re, decl_name, 0, NULL, 0);
}

static int32_t
should_run_decl(char const * restrict decl_name,
                char * restrict const * restrict decls_to_run,
                uint64_t ndecls_to_run,
                uint8_t * restrict decl_states,
                regex_t * restrict re_matchers) {
    uint64_t i;
    int32_t  has_hit;
    uint8_t  decl_state;

    if (!decls_to_run) {
        return 1;
    }
    has_hit = 0;
    for (i = 0; i < ndecls_to_run; ++i) {
        die_assert(decls_to_run[i] != NULL);
        decl_state = decl_states[i];

        if ((decl_state & decl_ran) && (decl_state & decl_did_one)) {
            continue;
        }
        if (!(decl_state & decl_re_built)) {
            decl_state |=
                (decl_re_built |
                 build_re_wildcard_matcher(re_matchers + i, decls_to_run[i]));
        }
        if (is_match(re_matchers + i, decl_name)) {
            dbg_assert(decl_states[i] != decl_do_nothing);

            decl_state |= decl_ran;
            has_hit = 1;
        }
        decl_states[i] = decl_state;
    }
    return has_hit;
}


void
list_decls(decl_list_t const * restrict decl_list) {
    uint64_t            i, ndecls;
    func_decl_t const * decls;
    die_assert(decl_list != NULL, "Error, trying to list null decls");

    ndecls = decl_list->ndecls;
    fprintf(stdout, "----------------- Listing %s ------------------\n",
            decl_list->decl_desc);

    for (i = 0; i < ndecls; ++i) {
        decls = decl_list->decls;

        die_assert(decls[i].name && decls[i].data,
                   "Error, unitialized decl struct!\n");
        fprintf(stdout, "%-24s\n", decls[i].name);
    }
    fprintf(stdout, "------------- Finished Listing %s -------------\n",
            decl_list->decl_desc);
}


void
run_decls(decl_list_t const * restrict decl_list,
          char * restrict const * restrict decls_to_run,
          uint64_t         ndecls_to_run,
          const run_decl_f run_decl_func) {
    uint64_t            i, ndecls;
    const func_decl_t * decls;
    regex_t             re_matchers[ndecls_to_run];
    uint8_t             decl_states[ndecls_to_run], decl_state;
    memset_c(decl_states, 0, ndecls_to_run);


    die_assert(decl_list != NULL, "Error, trying to list null decls");
    die_assert(run_decl_func != NULL, "Error, runner function is null");

    ndecls = decl_list->ndecls;
    decls  = decl_list->decls;
    for (i = 0; i < ndecls; ++i) {
        die_assert(decls[i].name && decls[i].data,
                   "Error, unitialized decl struct!\n");
        if (should_run_decl(decls[i].name, decls_to_run, ndecls_to_run,
                            decl_states, re_matchers)) {
            run_decl_func(decls + i);
        }
    }
    if (!decls_to_run) {
        return;
    }
    for (i = 0; i < ndecls_to_run; ++i) {
        die_assert(decls_to_run[i]);
        decl_state = decl_states[i];
        if (decl_state != decl_do_nothing) {

            die_assert(decl_state & decl_re_built);
            re_free(re_matchers + i);

            if (decl_state & decl_ran) {
                continue;
            }
        }
        fprintf(stdout, "Unable To Find - %s\n", decls_to_run[i]);
    }
}
