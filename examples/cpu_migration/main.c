#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"
#include "LINF/sym_all.h"
#include "L1/stack_switch.h"
#include "extension.h"
#include <dlfcn.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

//include for va_list
#include <stdarg.h>

#define GET_KERN_GS_CLOBBER_USER_GS                           \
  __asm__ __volatile__ (        \
    "movl $0xc0000102, %%ecx;" \
    "rdmsr;"                   \
    "movl $0xc0000101, %%ecx;" \
    "wrmsr;"                   \
    "sti;"                     \
    :: :"%rax", "%edx", "%ecx" \
    );


//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);

void dummy(void) {
    return;
}

int main() {
    // uint64_t flags = 0;
    printf("Starting cpu migration example\n");

    // flags = SYM_ELEVATE_FLAG | SYM_NOSMEP_FLAG | SYM_NOSMAP_FLAG | SYM_TOGGLE_SMEP_FLAG | SYM_TOGGLE_SMAP_FLAG;
    // sym_mode_shift(flags); //dont set interrupt disable flag
    // GET_KERN_GS_CLOBBER_USER_GS;
    sym_elevate();

    // //loop 10 times
    for(int i = 0; i < 100; i++) {

        void* task_struct = get_current_task_struct();
        //get current pid
        pid_t pid = get_current_pid();
        //get current cpu id
        int cpu_id = get_current_cpu_id();

        //read gs register
        uint64_t gs = get_current_gs_base();
        uint64_t cr3 = get_current_cr3();

        unsigned int symbi_state = get_current_symbi_state();
        unsigned int sym_elev = (symbi_state >> 1) & 0x1;
        unsigned int sym_mig = symbi_state & 0x1;

        _printk("Hello from process %d (%llx) on CPU %d, GS=0x%lx, CR3=0x%llx, sym_elev: %u, sym_mig: %u\n", pid, (unsigned long long)task_struct, cpu_id, gs, cr3, sym_elev, sym_mig); 

        //sleep for .5 seconds
        usleep(500000);
    }


    sym_lower();
    printf("DONE\n");
    return 0;
}

/*
set environment LD_BIND_NOW=1 
set environment LD_LIBRARY_PATH=/home/user/Symbi-OS/Symlib/dynam_build:.

sudo LD_BIND_NOW=1 LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./main
*/