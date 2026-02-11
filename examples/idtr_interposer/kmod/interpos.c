#include "common.h"
#include <linux/mm.h>



INTERPOSER_HANDLER(df_jmp_to_c, df_c_entry);


__attribute((unused)) void df_c_entry(struct pt_regs *pt_r){
  // Error code on DF is 0
  pt_r->orig_ax = USER_FT | WR_FT;
}


INTERPOSER_HANDLER(tf_jmp_to_c, tf_c_entry);

__attribute((unused)) void tf_c_entry(struct pt_regs *pt_r){
  // Stack or bust.
  /* char ins[] = "ins fetch\n"; */
  /* char wr[] = " wr\n"; */
  /* char str[] = "Stack\n"; */

  /* void (*tu_printk)(char *) = (void *)0xffffffff81c87049 ; */
  /* tu_printk(str); */


  // if(pt_r->orig_ax & WR_FT){
  //   return;
  // }

  pt_r->orig_ax |= USER_FT;

  /* /\* Are we an instruction fetch? *\/ */
  /* if(pt_r->error_code & INS_FETCH){ */
  /*   // We don't need to special case when in ring 3. */
  /*   if(! (pt_r->error_code & USER_FT)  ){ */
  /*     // Are we user code? */
  /*     // Could look in cr2, but by def rip caused the fault here. */
  /*     // This is modeled after kern:fault_in_kernel_space */
  /*     if(pt_r->rip < ( (1UL << 47) - 4096) ){ */
  /*       /// Lie that code was running in user mode. */
  /*       pt_r->error_code |= USER_FT; */
  /*       /\* tu_printk(ins); *\/ */
  /*       /\* print_ef(); *\/ */
  /*       /\* myprintki("my_ctr %d\n", my_ctr++); *\/ */
  /*     } */
  /*   } */
  /* } */
}


TRAP_HANDLER(int3_jmp_to_c, bp_c_entry);

// NOTE: This function is not used in C code, but is used in inline assembly.
// This asks the compiler not to warn about it being unused.
__attribute((unused)) void bp_c_entry(struct pt_regs *pt_r){
  // HACK: safe way is to generate pointer into pt_regs
  // This looks safe for first 3 args for now.
  // RAX def gets clobbered XXX

  // tcp_sendmsg

  // Assume we got here on an int3

  // NOTE: PC has moved past int3. Need it there again to replace byte
  // and to execute the instruction we replaced.
  pt_r->ip -= 1;
  unsigned char *ucp = (unsigned char *) pt_r->ip;
  /*   // assert that val we got here on was 0xcc, or int3 */
  while(*ucp != 0xcc);

  // Fixup instruction assuming it's an 0xf
  *ucp = 0xf;



  return;

  /* memcpy((void *)addr_msg, (void*)int3_rsi, 96); */

  // Our simulated memcpy
  // char *csrc = (char *)int3_rsi;
  // char *cdest = (char *)addr_msg;
  // int n = 96;
  // for (int i=0; i<n; i++)
  //   cdest[i] = csrc[i];

  // return;

  // myprintk("hey\n");

  // uint64_t my_cr3;
  // GET_CR3(my_cr3);
  // if(cr3_reg == my_cr3){
  //   myprintk("wow, cr3 matches\n");

  //   myprintk("Need to swing overwritten byte back to 0xf\n");
  //   unsigned char *ucp = (unsigned char *) 0xffffffff810fbfe0;
  //   *ucp = reset_byte;
  // }else{
  //   myprintk("Bummer, no match\n");
  // }
}


TRAP_HANDLER(db_jmp_to_c, db_c_entry);

__attribute((unused)) void db_c_entry(struct pt_regs *pt_r){
  struct DR7 dr7;
  GET_DR(7, dr7);

  struct DR6 dr6;
  GET_DR(6, dr6);

  uint64_t dr_hit = 9;

  uint64_t cr3 = 0;
  GET_CR3(cr3);

  // get RIP = hdl_pg address
  uint64_t hdl_pg;
  GET_PC(hdl_pg);

  uint64_t sp_ptr;
  struct scratchpad * sp;

  if(dr6.B0 && dr7.G0){
    dr_hit = 0;
    dr7.G0 = 0;
    dr7.RW0 = 0;
  }
  if(dr6.B1 && dr7.G1){
    dr_hit = 1;
    dr7.G1 = 0;
    dr7.RW1 = 0;
  }
  if(dr6.B2 && dr7.G2){
    dr_hit = 2;
    dr7.G2 = 0;
    dr7.RW2 = 0;
  }
  if(dr6.B3 && dr7.G3){
    dr_hit = 3;
    dr7.G3 = 0;
    dr7.RW3 = 0;
  }
  if(dr6.B0 && dr7.L0){
    dr_hit = 0;
    dr7.L0 = 0;
    dr7.RW0 = 0;
  }
  if(dr6.B1 && dr7.L1){
    dr_hit = 1;
    dr7.L1 = 0;
    dr7.RW1 = 0;
  }
  if(dr6.B2 && dr7.L2){
    dr_hit = 2;
    dr7.L2 = 0;
    dr7.RW2 = 0;
  }
  if(dr6.B3 && dr7.L3){
    dr_hit = 3;
    dr7.L3 = 0;
    dr7.RW3 = 0;
  }

  // align sp_ptr and get scratchpad pointer
  sp_ptr = (hdl_pg - (hdl_pg % PAGE_SIZE));
  sp_ptr += (PAGE_SIZE - 8);
  uint64_t * ptr = (uint64_t *) sp_ptr;
  sp = (struct scratchpad *)(*ptr);

  // copy some register values and what DB register was hit onto the scratchpad
  sp->get.dr_hit = dr_hit;
  sp->get.dr7 = dr7.val;
  sp->get.pt_r = *pt_r;
  sp->get.cr3 = cr3;

  if(sp->debug == 1){
    sp->cnt = sp->cnt + 1;
  }

  if(sp->read_addr_msg){
    char *csrc = (char *)pt_r->si;
    char *cdest = (char *)sp->addr_msg;
    int n = 96;
    for (int i=0; i<n; i++)
      cdest[i] = csrc[i];
  }

  SET_DR(7,dr7);

  return;
}