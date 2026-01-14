#include <linux/types.h>
#include <asm/ptrace.h>
#include <linux/objtool.h>

#define PRESENT 1
#define WR_FT   1<<1
#define USER_FT 1<<2
#define INS_FETCH 1<<4


//this might become problematic in the kernel
#define NEW_HANDLER(FN)                      \
  __asm__(".text \n\t .align 16 \n\t .globl \t" #FN "\n\t" #FN ":");

#define CALL_TARG(FN) \
__asm__("call " #FN);
#define IRET __asm__("iretq");


#define RET_TO_PG_FT                            \
  __asm__(" push   %rax                     \n\t\
  mov    $0xffffffff81001290,%rax           \n\t\
  xor    (%rsp),%rax /*Arlo's trick*/       \n\t\
  xor    %rax,(%rsp)                        \n\t\
  xor    (%rsp),%rax                        \n\t\
  ret                                           \
");

#define GET_PT_REG_PTR  __asm__("movq %rsp, %rdi");


#define PUSH_REGS \
  __asm__("\
  pushq   %rdi		/* pt_regs->di */ \n\t\
  pushq   %rsi		/* pt_regs->si */ \n\t\
  pushq	  %rdx		/* pt_regs->dx */ \n\t\
  pushq   %rcx		/* pt_regs->cx */ \n\t\
  pushq   %rax		/* pt_regs->ax */ \n\t\
  pushq   %r8		/* pt_regs->r8 */ \n\t\
  pushq   %r9		/* pt_regs->r9 */ \n\t\
  pushq   %r10		/* pt_regs->r10 */ \n\t\
  pushq   %r11		/* pt_regs->r11 */ \n\t\
  pushq	  %rbx		/* pt_regs->rbx */ \n\t\
  pushq	  %rbp		/* pt_regs->rbp */ \n\t\
  pushq	  %r12		/* pt_regs->r12 */ \n\t\
  pushq	  %r13		/* pt_regs->r13 */ \n\t\
  pushq	  %r14		/* pt_regs->r14 */ \n\t\
  pushq	  %r15		/* pt_regs->r15 */ \
");


#define POP_REGS \
__asm__("\
	popq %r15 \n\t\
	popq %r14 \n\t\
	popq %r13 \n\t\
	popq %r12 \n\t\
	popq %rbp \n\t\
	popq %rbx \n\t\
	popq %r11 \n\t\
	popq %r10 \n\t\
	popq %r9  \n\t\
	popq %r8  \n\t\
	popq %rax \n\t\
	popq %rcx \n\t\
	popq %rdx \n\t\
	popq %rsi \n\t\
	popq %rdi \
");



#define INTERPOSER_HANDLER(LAB, TARG)      \
  NEW_HANDLER(LAB)                              \
    PUSH_REGS                                   \
    GET_PT_REG_PTR                              \
    CALL_TARG(TARG)                             \
    POP_REGS                                    \
    RET_TO_PG_FT




#define GET_CR3(VAR) __asm__("movq %%cr3,%0" : "=r"( VAR ));


#define GET_DR(reg_num, var_name) \
asm("mov %%db"#reg_num " , %0" : "=r"(var_name));

#define SET_DR(reg_num, var_name)                   \
  asm("mov %0,%%db"#reg_num :: "r"(var_name));

#define GET_PC(var_name)         \
  asm volatile (\
    ANNOTATE_INTRA_FUNCTION_CALL \
    "call here2\n\t" \
                "here2:\n\t"     \
                "pop %0"         \
                : "=m" (hdl_pg));



#define PUSH_FAKE_ERROR __asm__("push $0xbadbad");
#define DROP_FAKE_ERROR __asm__("add $8, %rsp");

#define TRAP_HANDLER(LAB, TARG)                \
  NEW_HANDLER(LAB)                             \
    PUSH_FAKE_ERROR                               \
    PUSH_REGS                                  \
    GET_PT_REG_PTR                                \
    CALL_TARG(TARG)                              \
    POP_REGS                                   \
    DROP_FAKE_ERROR                               \
    IRET

struct rs_struct {
  struct pt_regs pt_r;
  uint64_t dr_hit;
  uint64_t dr7;
  uint64_t cr3;
};

struct scratchpad {
  struct rs_struct get;
  struct rs_struct set;
  struct rs_struct control;
  uint8_t debug;
  uint8_t cnt;
  uint8_t read_addr_msg;
  char addr_msg[96];
};


struct DR6 {
  union {
    uint64_t val;
    struct {
      uint64_t B0 : 1, B1 : 1, B2 : 1, B3 : 1, Res : 9, BD : 1, BS : 1, BT : 1;
    };
  }__attribute__((packed));
};
static_assert(sizeof(struct DR6) == 8, "Size of DR6 is not correct");

struct DR7 {
  union {
    uint64_t val;
    struct {
      uint64_t L0 : 1, G0 : 1, L1 : 1, G1 : 1, L2 : 1, G2 : 1, L3 : 1, G3 : 1,
          LE : 1, GE : 1, : 3, GD : 1, : 2, RW0 : 2, LEN0 : 2, RW1 : 2,
          LEN1 : 2, RW2 : 2, LEN2 : 2, RW3 : 2, LEN3 : 2, : 32;
    };
  }__attribute__((packed));
};
static_assert(sizeof(struct DR7) == 8, "Size of DR7 is not correct");