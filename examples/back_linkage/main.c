#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"
#include "LINF/sym_all.h"
#include "L1/stack_switch.h"
#include "extension.h"
#include <dlfcn.h>

//include for va_list
#include <stdarg.h>

extern uintptr_t cpu_current_top_of_stack;
//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);

int user_add(int a, int b) {
    // printf("user_add: %d + %d = %d\n", a, b, a + b);
    return a + b;
}

void user_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

int main() {
    volatile void * _printk_ptr;
    _printk_ptr = (void *)_printk;
    uintptr_t ktos = (uintptr_t)&cpu_current_top_of_stack;

    printf("main: calling elevate: %p \n", _printk_ptr);
    printf("main: cpu_current_top_of_stack addr: %p \n", &cpu_current_top_of_stack);
    printf("main: user_printf addr: %p \n", &user_printf);
    user_printf("user_printf: hello from user_printf!, test: %d\n", 123);

    sym_elevate();
    // _printk("hello world\n");
    printf("main: called elevate\n");
    //run kernel_add
    int sum = kernel_add(3, 4);
    printf("printf: kernel_add(3, 4) = %d\n", sum);
    // _printk("printk: kernel_add(3, 4) = %d\n", sum);

    //run current_pid
    int pid = current_pid();
    printf("printf: current_pid() = %d\n", pid);
    // _printk("printk: current_pid() = %d\n", pid);

    //force linkage of user_add
    // sym_lower();
    // void* dlsym_ret = dlsym(RTLD_DEFAULT, "user_add");
    // printf("main: attempting to call user_add via dlsym, dlsym: %p, &dlsym: %p, dlsym_ret: %p\n", dlsym, &dlsym, dlsym_ret);
    // SYM_ON_KERN_STACK_DYNSYM_DO(ktos, dyn_link_example(dlsym, RTLD_DEFAULT));
    // int userSum = user_add(5, 7);
    // printf("printf: user_add(5, 7) = %d\n", userSum);
    // sym_elevate();
    // _printk("printk: user_add(5, 7) = %d\n", userSum);

    
    sum = kernel_user_add(10, 15);
    _printk("printk: kernel_user_add(10, 15) = %d\n", sum);
    sym_lower();
    
    printf("printf: kernel_user_add(10, 15) = %d\n", sum);
    
    


    
    printf("DONE\n");
    return 0;
}

/*
set environment LD_BIND_NOW=1 
set environment LD_LIBRARY_PATH=/home/user/Symbi-OS/Symlib/dynam_build:.

sudo LD_BIND_NOW=1 LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./main
*/