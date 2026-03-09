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
#include <fcntl.h>

//include for va_list
#include <stdarg.h>

#define MODE_SLEEP 1
#define MODE_BUSY 2
#define MODE_IO 3

#define BUFFER_SIZE (1024 * 1024)  // 1MB buffer for I/O operations

//native kernel function that will be resolved at load time
extern int _printk(const char *fmt, ...);

void dummy(void) {
    return;
}

// Busy computation: calculate prime numbers
void busy_work(void) {
    volatile uint64_t count = 0;
    uint64_t limit = 100000;
    
    for (uint64_t num = 2; num < limit; num++) {
        int is_prime = 1;
        for (uint64_t i = 2; i * i <= num; i++) {
            if (num % i == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime) {
            count++;
        }
    }
}

// Blocking I/O: write and read from a file
void io_work(void) {
    static char buffer[BUFFER_SIZE];
    static int initialized = 0;
    
    if (!initialized) {
        // Fill buffer with some data
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = (char)(i % 256);
        }
        initialized = 1;
    }
    
    // Write to file
    int fd = open("/tmp/cpu_migration_test.dat", O_WRONLY | O_CREAT | O_SYNC, 0644);
    if (fd >= 0) {
        ssize_t written = write(fd, buffer, BUFFER_SIZE);
        fsync(fd);  // Force sync to ensure blocking I/O
        close(fd);
    }
    
    // Read from file
    fd = open("/tmp/cpu_migration_test.dat", O_RDONLY | O_SYNC);
    if (fd >= 0) {
        ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
        close(fd);
    }
}

// Sleep work
void sleep_work(void) {
    usleep(500000);  // 0.5 seconds
}

int main(int argc, char *argv[]) {
    int mode = MODE_SLEEP;
    
    if (argc > 1) {
        if (strcmp(argv[1], "sleep") == 0) {
            mode = MODE_SLEEP;
        } else if (strcmp(argv[1], "busy") == 0) {
            mode = MODE_BUSY;
        } else if (strcmp(argv[1], "io") == 0) {
            mode = MODE_IO;
        } else {
            printf("Unknown mode '%s', defaulting to sleep\n", argv[1]);
        }
    }
    
    const char *mode_name[] = {"", "Sleep", "Busy", "I/O"};
    printf("Running in %s mode\n\n", mode_name[mode]);

    sym_elevate();

    // Loop 100 times
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

        // Perform work based on selected mode
        switch(mode) {
            case MODE_SLEEP:
                sleep_work();
                break;
            case MODE_BUSY:
                busy_work();
                break;
            case MODE_IO:
                io_work();
                break;
        }
    }

    sym_lower();
    printf("DONE\n");
    
    // Clean up temporary file if in I/O mode
    if (mode == MODE_IO) {
        unlink("/tmp/cpu_migration_test.dat");
    }
    
    return 0;
}

/*
set environment LD_BIND_NOW=1 
set environment LD_LIBRARY_PATH=/home/user/Symbi-OS/Symlib/dynam_build:.

sudo LD_BIND_NOW=1 LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./main
*/