#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"

#include "load_mod.h"

//DECLARE_IFUNC(name, rettype, args)
//these functions are defined in the kernel module we will load
DECLARE_IFUNC(print_idt_entries, void, (void)) 
DECLARE_IFUNC(setup_df_interposition, void, (void))


//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);
extern uintptr_t ktos;

int main() {


    //run print_idt_entries
    // SYM_ON_KERN_STACK_DYNSYM_DO(ktos, print_idt_entries());
    // printf("printf: called print_idt_entries()\n");
    {
        SYM_ON_KERN_STACK_DYNSYM_DO(ktos, setup_df_interposition());
        printf("printf: called setup_df_interposition()\n");
    }
    
    {
        // print_idt_entries();
        SYM_ON_KERN_STACK_DYNSYM_DO(ktos, print_idt_entries());
        printf("printf: called print_idt_entries() again\n");
    }
    
    printf("DONE\n");
    return 0;
}

/*
set environment LD_BIND_NOW=1 
set environment LD_LIBRARY_PATH=/home/user/Symbi-OS/Symlib/dynam_build:.

sudo LD_BIND_NOW=1 LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./main
*/