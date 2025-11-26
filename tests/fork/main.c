#include <stdio.h>
#include <string.h>
#include "L0/sym_lib.h"

#include "load_mod.h"

//DECLARE_IFUNC(name, rettype, args)
//these functions are defined in the kernel module we will load
DECLARE_IFUNC(print_idt_entries, void, (void)) 


//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);


int main() {

    //fork once, child will print and exit
    pid_t pid = fork();
    if (pid < 0) {
        printf("main: fork failed\n");
        return 1;
    }
    if (pid == 0) {
        //child
        printf("CHILD: in child process\n");
        return 0;
    }

    printf("main: calling elevate\n");
    sym_elevate();
    printf("main: called elevate\n");

    //fork again, child will printk, lower and exit
    pid = fork();
    if (pid < 0) {
        printf("main: fork failed\n");
        return 1;
    }
    if (pid == 0) {
        //child
        printf("CHILD: in elevated child process\n");
        _printk("printk: hello from elevated child process!\n");
        sym_lower();
        printf("CHILD: lowered privileges, exiting\n");
        return 0;
    }

    printf("main: about to lower\n");
    sym_lower();

    //fork again, child prints and exits
    pid = fork();
    if (pid < 0) {
        printf("main: fork failed\n");
        return 1;
    }
    if (pid == 0) {
        //child
        printf("CHILD: in lowered child process\n");
        return 0;
    }

    printf("DONE\n");
    return 0;
}

//sudo LD_BIND_NOW=1 LD_LIBRARY_PATH=../../Symlib/dynam_build:. ./main