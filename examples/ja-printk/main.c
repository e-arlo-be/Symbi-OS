#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"
#include <dlfcn.h>
#include <assert.h>
#include <inttypes.h>
#include "greeter.kh"
#include "pfadaptor.kh"

//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);


void back_linkage_example(void) {
    printf("This is a back linkage example.\n");
}

static inline unsigned long get_exc_page_fault_addr()
{
  unsigned long val;

  val = (unsigned long)dlsym(RTLD_DEFAULT, "asm_exc_page_fault");

    

#if 0
  __asm__ volatile (
		    "mov exc_page_fault, %0"
		    : "=r" (val)
		    :
		    : "memory" );
#endif
  return val;
}

int stacktouch(void)
{
  const int n=4096*8;
  char data[n];
  int sum;
  
  for (int i=0; i<n; i++) { sum+=data[i]; }
  
  return sum;
}


extern int overflowuid;

int main() {
    volatile void * _printk_ptr;
    _printk_ptr = (void *)_printk;
    unsigned long df_cnt, pf_cnt;
    
    printf("main: calling elevate: %p \n", _printk_ptr);

    printf("overflowuid=%p\n", &overflowuid);
    
    intptr_t  pfaddr  = (intptr_t)dlsym(RTLD_DEFAULT, "asm_exc_page_fault");
    intptr_t  dfaddr  = (intptr_t)dlsym(RTLD_DEFAULT, "asm_exc_double_fault");

    printf("pfaddr=0x%lx dfaddr=0x%lx\n", pfaddr, dfaddr);

    int ss;
    sym_elevate();
    ss = stacktouch();
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

    pf_cnt = pf_adaptor_pf_cnt_get();
    df_cnt = pf_adaptor_df_cnt_get();
    
    sym_lower();
    //   assert(pf == get_exc_page_fault_addr());

    printf("pf_cnt=%ld df_cnt=%ld ss=%d\n", pf_cnt, df_cnt,
	   ss);
    
    printf("DONE\n");
    return 0;
}
