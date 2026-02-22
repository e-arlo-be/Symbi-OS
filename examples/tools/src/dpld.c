#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include "LINF/sym_all.h"
#include "L1/stack_switch.h"

#define DECLARE_ALL_FUNCS
#include "app_got.h"

static int module_loaded = 0;
static int verbose       = 1;

extern unsigned long kallsyms_lookup_name(const char *name);
extern void* vmalloc_noprof(unsigned long size);

void (*set_app_got)(app_got_t* got);

#define VPRINTF(fmt, ...) do {					\
    if (verbose) { fprintf(stderr, fmt, ##__VA_ARGS__); }	\
  } while(0)

extern const uint8_t _binary_ext_ko_start[];
extern const uint8_t _binary_ext_ko_end[];
extern const uint8_t _binary_ext_ko_size;

// extern int load_module(struct load_info *info, char *uargs,int flags);
extern int __x64_sys_init_module(void* args);

//__x64_sys_init_module expects the arguments to be on the stack
void do_load_module(void* umod, unsigned long len, char* uargs, unsigned long pfaddr, unsigned long dfaddr,
		    unsigned long gpaddr, int* ret_out) {
    //prepare argument passing
    // mov    0x60(%rdi),%rdx
    // mov    0x68(%rdi),%rsi
    // mov    0x70(%rdi),%rdi
    
    uint64_t args[3];
    args[0] = (uint64_t)uargs; //rdx
    args[1] = (uint64_t)len; //rsi
    args[2] = (uint64_t)umod; //rdi

    int ret = __x64_sys_init_module((void*)(args) - 0x60);
    if (ret_out) {
        *ret_out = ret;
    }
    {
      int (*func)(unsigned long, unsigned long, unsigned long);
      func = (void *)kallsyms_lookup_name("pf_adaptor_init");
      // page fault adaptor to ensure faults to
      func(pfaddr, dfaddr, gpaddr);
    }
}

static int
resolve_sym(char *name, void **value)
{
  char *error;
  dlerror();  // clear as per manpage
  *value = dlsym(RTLD_DEFAULT, name);
  error = dlerror();
  if (error != NULL) {
    fprintf(stderr, "%s\n", error);
    return 0;
  }
  return 1;
}

static int
force_symres_now()
{
  void *value;
  // force symbol resolution before we elevate  
  // lookup symbols to avoid problems once elevated -- touch symbol tables
  if (!resolve_sym("__x64_sys_init_module", &value)) return 0;
  if (!resolve_sym("cpu_current_top_of_stack", &value)) return 0;
  if (!resolve_sym("kallsyms_lookup_name", &value)) return 0;
  if (!resolve_sym("vmalloc_noprof", &value)) return 0;
  if (!resolve_sym("exit", &value)) return 0; //does this go through the GOT?
  return 1;
}

//assume sym_elevate has been called before this function
int load_ext_module() {
  VPRINTF("starting load_ext_module\n");
  int ret = 0;
  size_t size = (size_t)&_binary_ext_ko_size;
  char * uargs = "name=Hansi";
  uintptr_t ktos;
  
  if (force_symres_now()==0) {
    fprintf(stderr, "ERROR: failed to resolve symbols needed\n");
    assert(0);
    return 0;
  }
  
  if (!resolve_sym("cpu_current_top_of_stack", (void **)&ktos)) {
    VPRINTF("failed to resolve cpu_current_top_of_stack\n");
    assert(0);
    return 0;
  }
  
  unsigned long pfaddr;
  if (!resolve_sym("asm_exc_page_fault", (void **)&pfaddr)) {
    VPRINTF("failed to resolve asm_exc_page_fault\n");
    assert(0);
    return 0;
  }
  unsigned long dfaddr;
  if (!resolve_sym("asm_exc_double_fault", (void **)&dfaddr)) {
    VPRINTF("failed to resolve asm_exc_double_fault\n");
    assert(0);
    return 0;
  }
  unsigned long gpaddr;
  if (!resolve_sym("asm_exc_general_protection", (void **)&gpaddr)) {
    VPRINTF("failed to resolve asm_exc_general_protection\n");
    assert(0);
    return 0;
  }
  
  VPRINTF("ktos=%lx pfaddr=%lx dfaddr=%lx gpaddr=%lx\n", ktos, pfaddr, dfaddr, gpaddr);
  
  VPRINTF("starting load_module\n");
  
  VPRINTF("do_load_module: umod=%p len=%lu uargs=%p\n", uargs, size, uargs);
  
  SYM_ON_KERN_STACK_DYNSYM_DO(ktos, 
			      do_load_module((void*)_binary_ext_ko_start,
					     size, uargs, pfaddr, dfaddr, gpaddr, &ret));
  
  VPRINTF("do_load_module: exited __x64_sys_init_module ret=%d\n", ret);
  
  VPRINTF("exited load_module ret=%d\n", ret);
  return ret;
}

//resolves a symbol by name, loading the module if necessary
//this is for symbols from the kernel module included in our fat binary
void* dpld_resolver(char* symbol_name) {
  VPRINTF("%s: Resolving symbol %s\n", __func__, symbol_name);
  
  if (!module_loaded) {
    int rc;

    rc = load_ext_module();
    if (rc != 0) {
      VPRINTF("Failed to load ext module: %d\n", rc);
      //      exit(1);
    }
    VPRINTF("Loaded kallsyms module\n");
    
    #ifndef NO_APP_GOT_ENTRIES
    sym_elevate();
    set_app_got = (void (*)(app_got_t *))kallsyms_lookup_name("set_app_got");;
    if (!set_app_got) {
      VPRINTF("Failed to resolve set_app_got symbol!\n");
      exit(1);
    }

    app_got_t* app_got = (app_got_t*)vmalloc_noprof(sizeof(app_got_t));
    VPRINTF("allocated app_got at %p\n", app_got);

    SET_ALL_GOT_ENTRIES();
    VPRINTF("Set GOT entries\n");
    
    set_app_got(app_got);
    VPRINTF("Set app got pointer in extension\n");
    sym_lower();
    #else
    VPRINTF("No GOT entries to set, skipping GOT setup\n");
    #endif
    
    
    module_loaded = 1;
  }
  
  sym_elevate();
  unsigned long addr = kallsyms_lookup_name(symbol_name);
  VPRINTF("Resolved symbol %s to address %p\n", symbol_name, (void*)addr);
  sym_lower();
  
  if (addr == 0) {
    VPRINTF("Symbol %s not found!\n", symbol_name);
    //exit program! We have a linkage problem
    exit(1);
  }
  return (void*)addr;
}
