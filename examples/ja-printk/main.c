#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"
#include "extension.h"

//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);

int main() {

    printf("main: calling elevate\n");
    sym_elevate();
    printf("main: called elevate\n");

    //run kernel_add
    int sum = kernel_add(3, 4);
    printf("printf: kernel_add(3, 4) = %d\n", sum);
    _printk("printk: kernel_add(3, 4) = %d\n", sum);

    //run current_pid
    int pid = current_pid();
    printf("printf: current_pid() = %d\n", pid);
    _printk("printk: current_pid() = %d\n", pid);
    sym_lower();
    
    printf("DONE\n");
    return 0;
}
