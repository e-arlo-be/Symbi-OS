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
	.global pf_adaptor_asm_fix_err_code_u
pf_adaptor_asm_fix_err_code_u:
	// NYI: Add tests to only fix error code if faulting process is elevated and
	//      address and fault type are sensible.  
	// x86 allows immediate and memory operands without using a register
	incq  (pf_adaptor_pf_cnt)
	orl   $ERRCODE_PF_USER, (%rsp)
	jmp   *pf_adaptor_asm_exc_page_fault
	// int3 to satifiy objtool desire for a trap after a speculative jump to try and improve security
	int3

	.text
	.align 16
	.global pf_adaptor_asm_fix_err_code_uw
pf_adaptor_asm_fix_err_code_uw:
	// used to fix double fault 
	// x86 allows immediate and memory operands without using a register
	incq  (pf_adaptor_df_cnt)
	orl   $(ERRCODE_PF_USER|ERRCODE_PF_WRITE), (%rsp)
	jmp   *pf_adaptor_asm_exc_page_fault	
	// int3 to satifiy objtool desire for a trap after a speculative jump to try and improve security
	int3
	
