#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"
#include "LINF/sym_all.h"
#include "L1/stack_switch.h"
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

void elevated_work()
{
  unsigned long cr3;
  unsigned long df_cnt, pf_cnt;
  //    sym_elevate();
  __asm__ __volatile__("movq %%cr3,%0" : "=r"( cr3 ));
  _printk("ELEVATED CR3: %lx\n", cr3);
    
  //  int ss = stacktouch();
  _printk("hello world\n");
  //  printf("main: called elevate\n");
  //run kernel_add
  int sum = kernel_add(3, 4);
  // printf("printf: kernel_add(3, 4) = %d\n", sum);
  _printk("printk: kernel_add(3, 4) = %d\n", sum);

  //run current_pid
  int pid = current_pid();
  // printf("printf: current_pid() = %d\n", pid);
  _printk("printk: current_pid() = %d\n", pid);

  pf_cnt = pf_adaptor_pf_cnt_get();
  df_cnt = pf_adaptor_df_cnt_get();

  //  printf("pf_cnt=%ld df_cnt=%ld\n", pf_cnt, df_cnt);
  _printk("pf_cnt=%ld df_cnt=%ld\n", pf_cnt, df_cnt);
  // sym_lower();
}

extern int overflowuid;

int main() {
    volatile void * _printk_ptr;
    _printk_ptr = (void *)_printk;
    
    printf("main: calling elevate: %p \n", _printk_ptr);

    printf("overflowuid=%p\n", &overflowuid);
    
    intptr_t  pfaddr   = (intptr_t)dlsym(RTLD_DEFAULT, "asm_exc_page_fault");
    intptr_t  dfaddr   = (intptr_t)dlsym(RTLD_DEFAULT, "asm_exc_double_fault");
    
    printf("pfaddr=0x%lx dfaddr=0x%lx\n", pfaddr, dfaddr);

#ifdef KSTACK
    unsigned long ktos = (unsigned long)dlsym(RTLD_DEFAULT, "cpu_current_top_of_stack");
    SYM_ON_KERN_STACK_DYNSYM_DO(ktos, elevated_work());
#else
    unsigned long cr3;
    unsigned long df_cnt, pf_cnt;

    sym_elevate();
    //    __asm__ __volatile__("cli" :);
    __asm__ __volatile__("movq %%cr3,%0" : "=r"( cr3 ));
    //_printk("ELEVATED CR3: %lx\n", cr3);
    printf("ELEVATED CR3: %lx\n", cr3);
    
    int ss = stacktouch();
    //_printk("main:called elevated\n");
    printf("main: called elevate\n");

    // test_pfasm();

    //run kernel_add
    int sum = kernel_add(3, 4);
    printf("printf: kernel_add(3, 4) = %d\n", sum);
    //_printk("printk: kernel_add(3, 4) = %d\n", sum);
    
    //run current_pid
    int pid = current_pid();
    //_printk("printk: current_pid() = %d\n", pid);
    printf("printf: current_pid() = %d\n", pid);
    
    pf_cnt = pf_adaptor_pf_cnt_get();
    df_cnt = pf_adaptor_df_cnt_get();
    
    //  _printk("pf_cnt=%ld df_cnt=%ld ss=%d\n", pf_cnt, df_cnt, ss);      
    printf("pf_cnt=%ld df_cnt=%ld ss=%d\n", pf_cnt, df_cnt, ss);
    //__asm__ __volatile__("sti" :);
    sym_lower();
#endif
    printf("DONE\n");
    return 0;
}
