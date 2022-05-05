#ifndef _SRC__ASM__ASM_COMMON_H_
#define _SRC__ASM__ASM_COMMON_H_


#define L(name)          L##name
#define NAME_LABEL(name) name##:

#define ENTRY_DEF(name)                                                        \
    .globl name;                                                               \
    .type  name, @function;                                                    \
    NAME_LABEL(name)                                                           \
    cfi_startproc;

#ifndef _CET_ENDBR
#define _CET_ENDBR NOP4;
#endif
#ifndef _CET_NOTRACK
#define _CET_NOTRACK NOP1;
#endif

#define ENTRY_END(name)                                                        \
    ENTRY_DEF(name)                                                            \
    _CET_ENDBR

#define END_DEF(name)                                                          \
    cfi_endproc;                                                               \
    .size name, .- name;

#define ALIGNARG(log2) (1 << (log2))
#define ENTRY_P2ALIGN(name, alignment)                                         \
    .align ALIGNARG(alignment);                                                \
    ENTRY_END(name)

#define PAGE_ALIGN_CODE .align 4096;
#ifndef ALIGN_ENTRY
#define ALIGN_ENTRY 0
#endif

#if ALIGN_ENTRY == 0
#define ENTRY(name) ENTRY_P2ALIGN(name, 4)

#elif ALIGN_ENTRY == 48
#define ENTRY_P2ALIGN(name)                                                    \
    PAGE_ALIGN_CODE;                                                           \
    nop;                                                                       \
    .align 32;                                                                 \
    nop;                                                                       \
    .align 16;                                                                 \
    ENTRY_END(name)

#else
#define ENTRY(name)                                                            \
    PAGE_ALIGN_CODE;                                                           \
    nop;                                                                       \
    .align ALIGN_ENTRY;                                                        \
    ENTRY_END(name)
#endif

#if ALIGN_ENTRY == 0
#define END(name) END_DEF(name)
#else
#define END(name)                                                              \
    END_DEF(name)                                                              \
    PAGE_ALIGN_CODE;
#endif

#define cfi_startproc              .cfi_startproc
#define cfi_endproc                .cfi_endproc
#define cfi_def_cfa(reg, off)      .cfi_def_cfa reg, off
#define cfi_def_cfa_register(reg)  .cfi_def_cfa_register reg
#define cfi_def_cfa_offset(off)    .cfi_def_cfa_offset off
#define cfi_adjust_cfa_offset(off) .cfi_adjust_cfa_offset off
#define cfi_offset(reg, off)       .cfi_offset reg, off
#define cfi_rel_offset(reg, off)   .cfi_rel_offset reg, off
#define cfi_register(r1, r2)       .cfi_register r1, r2
#define cfi_return_column(reg)     .cfi_return_column reg
#define cfi_restore(reg)           .cfi_restore reg
#define cfi_same_value(reg)        .cfi_same_value reg
#define cfi_undefined(reg)         .cfi_undefined reg
#define cfi_remember_state         .cfi_remember_state
#define cfi_restore_state          .cfi_restore_state
#define cfi_window_save            .cfi_window_save
#define cfi_personality(enc, exp)  .cfi_personality enc, exp
#define cfi_lsda(enc, exp)         .cfi_lsda enc, exp

#include "nops.h"

#endif
