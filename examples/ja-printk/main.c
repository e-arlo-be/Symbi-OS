#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"
#include "extension.h"
#include <dlfcn.h>
#include <assert.h>
#include <inttypes.h>

//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);

static inline uintptr_t get_exc_page_fault_addr()
{
  uintptr_t val;

  val = (uintptr_t)dlsym(RTLD_DEFAULT, "asm_exc_page_fault");
#if 0
  __asm__ volatile (
		    "mov exc_page_fault, %0"
		    : "=r" (val)
		    :
		    : "memory" );
#endif
  return val;
}

int main() {
    volatile void * _printk_ptr;
    _printk_ptr = (void *)_printk;

    printf("main: calling elevate: %p \n", _printk_ptr);

    
    sym_elevate();
    _printk("hello world\n");
    printf("main: called elevate\n");
    //run kernel_add
    int sum = kernel_add(3, 4);
    printf("printf: kernel_add(3, 4) = %d\n", sum);
    _printk("printk: kernel_add(3, 4) = %d\n", sum);

    //run current_pid
    int pid = current_pid();
    printf("printf: current_pid() = %d\n", pid);
    _printk("printk: current_pid() = %d\n", pid);

#if 1
    // run pf_hdlr_addr()
    uintptr_t  pf = (uintptr_t) pf_hdlr_addr();
    printf("pf: %" PRIxPTR "\n", pf);
#endif
    sym_lower();

    assert(pf == get_exc_page_fault_addr());
    
    printf("DONE\n");
    return 0;
}
