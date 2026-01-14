#include <generated/asm-offsets.h>

	// HW ERROR CODE is last item pushed on to stack by exception logic
	// bits are defined as follows for pagefault exception-- from intel manual
	.equ ERRCODE_PF_PROT,  1<<0
	.equ ERRCODE_PF_WRITE, 1<<1
	.equ ERRCODE_PF_USER,  1<<2
	.equ ERRCODE_PF_RSVD,  1<<3
	.equ ERRCODE_PF_INSTR, 1<<4	
	.equ ERRCODE_PF_PK,    1<<5
	.equ ERRCODE_PF_SHSTK, 1<<6
	.equ ERRCODE_PF_HLAT,  1<<7
	.equ ERRCODE_PF_SGX,   1<<15
	
	.text
	.align 16
	.global pf_adaptor_asm_fix_err_code
pf_adaptor_asm_fix_err_code:
	// NYI: Add tests to only fix error code if faulting process is elevated and
	//      address and fault type are sensible.  
	// x86 allows immediate and memory operands without using a register
	incq  (pf_adaptor_cnt)
	orl   $ERRCODE_PF_USER, (%rsp)
	jmp   *pf_adaptor_orig_pf_asm_entry
	// int3 to satifiy objtool desire for a trap after a speculative jump to try and improve security
	int3
