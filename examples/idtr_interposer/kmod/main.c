#include <linux/module.h>
#include <linux/init.h>

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/uaccess.h>
#include <linux/signal.h>
#include <linux/pid.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/smp.h>


void df_jmp_to_c(void);
void tf_jmp_to_c(void);
void int3_jmp_to_c(void);
void db_jmp_to_c(void);


// Descriptor table entry. For use with GDT, IDT
struct dtr
{
  uint16_t limit; // limit 2
  uint64_t base;   // base 8
}__attribute__((packed));
static_assert(sizeof(struct dtr) == 10, "Size of dtr is not correct");

//taken from https://elixir.bootlin.com/linux/v6.16/source/tools/testing/selftests/kvm/include/x86/processor.h#L1167
struct idt_entry {
	uint16_t offset0;
	uint16_t selector;
	uint16_t ist : 3;
	uint16_t : 5;
	uint16_t type : 4;
	uint16_t : 1;
	uint16_t dpl : 2;
	uint16_t p : 1;
	uint16_t offset1;
	uint32_t offset2; uint32_t reserved;
};

struct dtr old_idtr;

//  Define the module metadata.
#define MODULE_NAME "greeter"
MODULE_AUTHOR("Dave Kerr");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple kernel module to greet a user");
MODULE_VERSION("0.1");


void print_idt_entries(void) {
    struct dtr idtr;
    struct idt_entry *idt;
    
    asm volatile ("sidt %0" : "=m" (idtr));

    idt = (struct idt_entry *)idtr.base;
    printk("IDT base address: 0x%016llx\n", (unsigned long long)idt);
    
    for (unsigned int i = 0; i < 256; i++) {
        uint64_t offset = ((uint64_t)idt[i].offset2 << 32) |
                          ((uint64_t)idt[i].offset1 << 16) |
                          (uint64_t)idt[i].offset0;
        uint16_t selector = idt[i].selector;
        uint8_t ist = idt[i].ist;
        uint8_t type_attr = (idt[i].type & 0x0F) | ((idt[i].dpl & 0x03) << 5) | ((idt[i].p & 0x01) << 7);
        
        printk("IDT Entry %3u: Offset=0x%016llx Selector=0x%04x IST=%u TypeAttr=0x%02x\n",
                i, offset, selector, ist, type_attr);
    }
}


struct idt_entry * duplicate_idt(void) {
    struct dtr idtr;
    struct idt_entry *original_idt;
    
    asm volatile ("sidt %0" : "=m" (idtr));

    original_idt = (struct idt_entry *)idtr.base;

    struct idt_entry *new_idt = kmalloc_array(256, sizeof(struct idt_entry), GFP_KERNEL);
    if (!new_idt) {
        printk("Failed to allocate memory for new IDT\n");
        return NULL;
    }

    memcpy(new_idt, original_idt, 256 * sizeof(struct idt_entry));
    printk("Duplicated IDT to new location: 0x%016llx\n", (unsigned long long)new_idt);
    return new_idt;
}

void set_idt_entry(struct idt_entry *idt, int vector, void *handler_addr) {
    uint64_t offset = (uint64_t)handler_addr;
    idt[vector].offset0 = offset & 0xFFFF;
    idt[vector].offset1 = (offset >> 16) & 0xFFFF;
    idt[vector].offset2 = (offset >> 32) & 0xFFFFFFFF;
}

// Function to load IDT on a single CPU
void load_idt_on_cpu(void *info) {
    struct dtr *idtr = (struct dtr *)info;
    asm volatile ("lidt %0" : : "m" (*idtr));
    printk("Loaded new IDT on CPU %d\n", smp_processor_id());
}

void setup_df_interposition(void) {
    struct idt_entry *new_idt = duplicate_idt();


    void *new_df_handler_addr = (void *)&df_jmp_to_c;
    set_idt_entry(new_idt, 8, new_df_handler_addr);
    printk("Modified DF handler to point to %016llx\n", (unsigned long long)new_df_handler_addr);
    

    void * new_tf_handler_addr = (void *)&tf_jmp_to_c;
    set_idt_entry(new_idt, 14, new_tf_handler_addr);
    printk("Modified TF handler to point to %016llx\n", (unsigned long long)new_tf_handler_addr);
  
  
    void *new_int3_handler_addr = (void *)&int3_jmp_to_c;
    set_idt_entry(new_idt, 3, new_int3_handler_addr);
    printk("Modified I3 handler to point to %016llx\n", (unsigned long long)new_int3_handler_addr);
    

    void * new_db_handler_addr = (void *)&db_jmp_to_c;
    set_idt_entry(new_idt, 1, new_db_handler_addr);
    printk("Modified DB handler to point to %016llx\n", (unsigned long long)new_db_handler_addr);
    

    // Load the new IDT on all CPUs
    struct dtr new_idtr;
    new_idtr.base = (uint64_t)new_idt;
    new_idtr.limit = sizeof(struct idt_entry) * 256 - 1;
    
    // Execute load_idt_on_cpu on all CPUs
    on_each_cpu(load_idt_on_cpu, &new_idtr, 1);
    printk("Loaded new IDT on all CPUs\n");
}









//  Define the name parameter.
static char *name = "Bilbo";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");


static int __init greeter_init(void)
{
    pr_info("%s: module loaded at 0x%p\n", MODULE_NAME, greeter_init);
    pr_info("%s: greetings %s\n", MODULE_NAME, name);

    //print reference to df_jmp_to_c
    pr_info("%s: df_jmp_to_c is at 0x%p\n", MODULE_NAME, df_jmp_to_c);

    //save original idtr
    asm volatile ("sidt %0" : "=m" (old_idtr));

    return 0;
}

static void __exit greeter_exit(void)
{
    pr_info("%s: goodbye %s\n", MODULE_NAME, name);
    pr_info("%s: module unloaded from 0x%p\n", MODULE_NAME, greeter_exit);


    //restore original idtr
    asm volatile ("lidt %0" : : "m" (old_idtr));
}

module_init(greeter_init);
module_exit(greeter_exit);
