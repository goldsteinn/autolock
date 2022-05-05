#ifndef _SRC__ARCH__X86__LL_SYSCALL_H_
#define _SRC__ARCH__X86__LL_SYSCALL_H_

#include "util/macro.h"
#include "util/types.h"

#define I_x86_internal_syscall(reg_setup, reg_decls...)                        \
    ({                                                                         \
        DEPAREN(reg_setup);                                                    \
        __asm__ volatile("syscall"                                             \
                         : "+r"(_rax)                                          \
                         : reg_decls                                           \
                         : "memory", "r11", "rcx");                            \
        _rax;                                                                  \
    })

#define I_x86_internal_syscall_cc(reg_setup, reg_decls, rw_clobbers,           \
                                  rd_clobbers, w_clobbers)                     \
    ({                                                                         \
        DEPAREN(reg_setup);                                                    \
        __asm__ volatile("syscall"                                             \
                         : "+r"(_rax)DEPAREN(rw_clobbers) DEPAREN(w_clobbers)  \
                         : DEPAREN(reg_decls) DEPAREN(rd_clobbers)             \
                         : "r11", "rcx");                                      \
        _rax;                                                                  \
    })


#define I_x86_internal_pin_reg(reg, val)                                       \
    register get_type(val) CAT(_, reg) __asm__(V_TO_STR(reg)) = (val)
#define I_x86_internal_pin_reg1(SYS_num) I_x86_internal_pin_reg(rax, SYS_num)
#define I_x86_internal_pin_reg2(SYS_num, arg0)                                 \
    I_x86_internal_pin_reg1(SYS_num);                                          \
    I_x86_internal_pin_reg(rdi, arg0)
#define I_x86_internal_pin_reg3(SYS_num, arg0, arg1)                           \
    I_x86_internal_pin_reg2(SYS_num, arg0);                                    \
    I_x86_internal_pin_reg(rsi, arg1)
#define I_x86_internal_pin_reg4(SYS_num, arg0, arg1, arg2)                     \
    I_x86_internal_pin_reg3(SYS_num, arg0, arg1);                              \
    I_x86_internal_pin_reg(rdx, arg2)
#define I_x86_internal_pin_reg5(SYS_num, arg0, arg1, arg2, arg3)               \
    I_x86_internal_pin_reg4(SYS_num, arg0, arg1, arg2);                        \
    I_x86_internal_pin_reg(r10, arg3)
#define I_x86_internal_pin_reg6(SYS_num, arg0, arg1, arg2, arg3, arg4)         \
    I_x86_internal_pin_reg5(SYS_num, arg0, arg1, arg2, arg3);                  \
    I_x86_internal_pin_reg(r8, arg4)
#define I_x86_internal_pin_reg7(SYS_num, arg0, arg1, arg2, arg3, arg4, arg5)   \
    I_x86_internal_pin_reg6(SYS_num, arg0, arg1, arg2, arg3, arg4);            \
    I_x86_internal_pin_reg(r9, arg5)

#define I_x86_internal_pin_regN(N, ...)                                        \
    CAT(I_x86_internal_pin_reg, N)(__VA_ARGS__)

#define I_x86_internal_reg1
#define I_x86_internal_reg2 "r"(_rdi)
#define I_x86_internal_reg3 I_x86_internal_reg2, "r"(_rsi)
#define I_x86_internal_reg4 I_x86_internal_reg3, "r"(_rdx)
#define I_x86_internal_reg5 I_x86_internal_reg4, "r"(_r10)
#define I_x86_internal_reg6 I_x86_internal_reg5, "r"(_r8)
#define I_x86_internal_reg7 I_x86_internal_reg6, "r"(_r9)


#define I_x86_add_rw_clobber(mem) , "+m"(*(mem))
#define I_x86_add_rd_clobber(mem) , "m"(*(mem))
#define I_x86_add_w_clobber(mem)  , "=m"(*(mem))

#define I_x86_add_clobbers_(clobber, ...) APPLY(clobber, EMPTY(), __VA_ARGS__)
#define I_x86_add_clobbers(clobber, ...)                                       \
    (I_x86_add_clobbers_(clobber, __VA_ARGS__))

#define I_x86_add_rw_clobbers(to_clobber)                                      \
    I_x86_add_clobbers(I_x86_add_rw_clobber, DEPAREN(to_clobber))
#define I_x86_add_rd_clobbers(to_clobber)                                      \
    I_x86_add_clobbers(I_x86_add_rd_clobber, DEPAREN(to_clobber))
#define I_x86_add_w_clobbers(to_clobber)                                       \
    I_x86_add_clobbers(I_x86_add_w_clobber, DEPAREN(to_clobber))


#define I_x86_internal_syscallN(N, ...)                                        \
    I_x86_internal_syscall(I_x86_internal_pin_regN(N, __VA_ARGS__),            \
                           CAT(I_x86_internal_reg, N))

#define I_x86_internal_syscallN_cc(N, vars, rw_clobbers, rd_clobbers,          \
                                   w_clobbers)                                 \
    I_x86_internal_syscall_cc(I_x86_internal_pin_regN(N, DEPAREN(vars)),       \
                              (CAT(I_x86_internal_reg, N)), rw_clobbers,       \
                              rd_clobbers, w_clobbers)

#define ll_syscall(...)                                                        \
    I_x86_internal_syscallN(PP_NARG(__VA_ARGS__), __VA_ARGS__)

#define I_ll_syscall_cc_combined(vars, rw_mem, rd_mem, w_mem)                  \
    I_x86_internal_syscallN_cc(                                                \
        PP_NARG(DEPAREN(vars)), vars, I_x86_add_rw_clobbers(rw_mem),           \
        I_x86_add_rd_clobbers(rd_mem), I_x86_add_w_clobbers(w_mem))

#define ll_syscall_cc(sys_num, vars, rw_mem, rd_mem, w_mem)                    \
    I_ll_syscall_cc_combined((ADD_ARG_FRONT(sys_num, DEPAREN(vars))), rw_mem,  \
                             rd_mem, w_mem)


#endif
