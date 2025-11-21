//******************************************************************************
// Copyright (c) 2018, The Regents of the University of California (Regents).
// All Rights Reserved. See LICENSE for license details.
//------------------------------------------------------------------------------
#include "mm/mm.h"
#include "util/rt_util.h"
#include "util/printf.h"
#include "uaccess.h"
#include "mm/vm.h"

// Statically allocated copy-buffer
unsigned char rt_copy_buffer_1[RISCV_PAGE_SIZE];
unsigned char rt_copy_buffer_2[RISCV_PAGE_SIZE];

size_t rt_util_getrandom(void* vaddr, size_t buflen){
  size_t remaining = buflen;
  uintptr_t rnd;
  uintptr_t* next = (uintptr_t*)vaddr;
  // Get data
  while(remaining > sizeof(uintptr_t)){
    rnd = sbi_random();
    ALLOW_USER_ACCESS( *next = rnd );
    remaining -= sizeof(uintptr_t);
    next++;
  }
  // Cleanup
  if( remaining > 0 ){
    rnd = sbi_random();
    copy_to_user(next, &rnd, remaining);
  }
  size_t ret = buflen;
  return ret;
}

void rt_util_misc_fatal(){
  //Better hope we can debug it!
  sbi_exit_enclave(-1);
}

void not_implemented_fatal(struct encl_ctx* ctx){
#ifdef FATAL_DEBUG
    unsigned long addr, cause, pc;
    pc = ctx->regs.sepc;
    addr = ctx->sbadaddr;
    cause = ctx->scause;
    printf("[runtime] non-handlable interrupt/exception at 0x%lx on 0x%lx (scause: 0x%lx)\r\n", pc, addr, cause);
#endif

    // Bail to m-mode
    __asm__ volatile("csrr a0, scause\r\nli a7, 1111\r\n ecall");

    return;
}

void rt_page_fault(struct encl_ctx* ctx)
{
#ifdef FATAL_DEBUG
  unsigned long addr, cause, pc;
  pc = ctx->regs.sepc;
  addr = ctx->sbadaddr;
  cause = ctx->scause;
  printf("[runtime] page fault at 0x%lx on 0x%lx (scause: 0x%lx)\r\n", pc, addr, cause);
  printf("[runtime]  ra: 0x%016lx  sp :0x%016lx\r\n", ctx->regs.ra, ctx->regs.sp);
  printf("[runtime]  a0 :0x%016lx  s0 :0x%016lx t0 :0x%016lx\r\n", ctx->regs.a0, ctx->regs.s0, ctx->regs.t0);
  printf("[runtime]  a1 :0x%016lx  s1 :0x%016lx t1 :0x%016lx\r\n", ctx->regs.a1, ctx->regs.s1, ctx->regs.t1);
  printf("[runtime]  a2 :0x%016lx  s2 :0x%016lx t2 :0x%016lx\r\n", ctx->regs.a2, ctx->regs.s2, ctx->regs.t2);
  printf("[runtime]  a3 :0x%016lx  s3 :0x%016lx t3 :0x%016lx\r\n", ctx->regs.a3, ctx->regs.s3, ctx->regs.t3);
  printf("[runtime]  a4 :0x%016lx  s4 :0x%016lx t4 :0x%016lx\r\n", ctx->regs.a4, ctx->regs.s4, ctx->regs.t4);
  printf("[runtime]  a5 :0x%016lx  s5 :0x%016lx t5 :0x%016lx\r\n", ctx->regs.a5, ctx->regs.s5, ctx->regs.t5);
  printf("[runtime]  a6 :0x%016lx  s6 :0x%016lx t6 :0x%016lx\r\n", ctx->regs.a6, ctx->regs.s6, ctx->regs.t6);
  printf("[runtime]  a7 :0x%016lx  s7 :0x%016lx s8 :0x%016lx\r\n", ctx->regs.s7, ctx->regs.s8);
  printf("[runtime]  s8 :0x%016lx  s9 :0x%016lx s10:0x%016lx\r\n", ctx->regs.s8, ctx->regs.s9, ctx->regs.s10);
  printf("[runtime]  s11:0x%016lx\r\n", ctx->regs.s11, ctx->regs.s7);
#endif

  sbi_exit_enclave(-1);

  /* never reach here */
  assert(false);
  return;
}

void tlb_flush(void)
{
  __asm__ volatile("fence.i\t\nsfence.vma\t\n");
}
