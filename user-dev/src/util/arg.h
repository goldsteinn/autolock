#ifndef _ARG_H_
#define _ARG_H_

/* simple argument parsing with documentation

 options can be typed as int, char, char*, bool, float, or handled by a
 special function positional parameters are always string and must be listed
 in order a special type Rest means all rest are returned as a pointer.  */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "util/attrs.h"

/* don't change constants unless you fix lots of other stuff.  */
typedef enum {
    Integer   = 1,
    Character = 2,
    String    = 3,
    Boolean   = 4,
    Float     = 5,
    Rest      = 6,
    Help,
    Increment,
    Set,
    Toggle,
    EndOptions /* use this to allow user to indicate end of positional args,
                  typically it will be for -- */
} ArgType;

/* don't renumber these.  We depend on value of KindPositional >
   (KindOption & KindHelp), KindEnd > (all), KindRest > KindPositional.  */
typedef enum {
    KindEnd        = 4,
    KindHelp       = 1,
    KindPositional = 2,
    KindOption     = 0,
    KindRest       = 3
} ArgKind;

#define ArgGetDefault -1
#define ArgGetDesc    -2

typedef struct arg_rest {
    char * const * ptrs;
    size_t         n;
} __attribute__((may_alias)) arg_rest_t;

#define INIT_ARG_REST_T                                                        \
    { NULL, 0 }

typedef struct argoption_ ArgOption;
struct argoption_ {
    ArgKind            kind;          /* option, positionl, rest, end, help */
    ArgType            type;          /* type of the argument/option */
    char const * const args_begin[4]; /* also name of positional arg */
    int32_t            required;
    void *             dest;
    uint64_t           dest_sz;     /* hidden to used. */
    int32_t            is_unsigned; /* hidden to used. */
    char const *       desc;
};


#define ADD_ARG(kind, type, names, reqd, var, help)                            \
    {                                                                          \
        kind, type, { DEPAREN(names) }, reqd, var, sizeof_deref(var),          \
            IS_UNSIGNED_INT(var), help                                         \
    }


typedef struct argdef_ ArgDefs;

struct argdef_ {
    /* public members.  */
    ArgOption *  args;
    char const * progdoc;
    char const * version;
    void (*doneParsing)(void);
};

int32_t parseArgs(int32_t        argc,
                  char * const * argv,
                  ArgDefs const * restrict def);

/* for more complicated arg parsing, i.e., including parsers of submodules, make
 * an ArgParser, then call parse on it.  */

typedef struct _argparserlist ArgParserNode;
struct _argparserlist {
    int32_t         main;
    ArgDefs const * parser;
    ArgParserNode * next;
};

typedef struct _argParser ArgParser;
struct _argParser {
    ArgParserNode * parsers;
    ArgDefs const * mainProg;
};

ArgParser * createArgumentParser(ArgDefs const * def);
void        freeArgumentParser(ArgParser * ap);
void        addArgumentParser(ArgParser * restrict ap,
                              ArgDefs const * restrict def,
                              int32_t order);
int32_t     parseArguments(ArgParser * restrict ap,
                           int32_t        argc,
                           char * const * argv);
void        freeCommandLine(void);
int32_t doParse(ArgDefs const * restrict argp, int argc, char * const * argv);

#endif
