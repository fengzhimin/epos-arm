/**
 * vim: filetype=c:fenc=utf-8:ts=4:et:sw=4:sts=4
 *
 * Copyright (C) 2005, 2008, 2013 Hong MingJian<hongmingjian@gmail.com>
 * All rights reserved.
 *
 * This file is part of the EPOS.
 *
 * Redistribution and use in source and binary forms are freely
 * permitted provided that the above copyright notice and this
 * paragraph and the following disclaimer are duplicated in all
 * such forms.
 *
 * This software is provided "AS IS" and without any express or
 * implied warranties, including, without limitation, the implied
 * warranties of merchantability and fitness for a particular
 * purpose.
 *
 */
#ifndef _CPU_H
#define _CPU_H

#include <inttypes.h>


static __inline void cli()
{
    asm volatile(
      "mrs r0, cpsr\n\t"
    	"bic r0, r0, #0x80\n\t"
    	"msr cpsr_c, r0\n\t"
    	"mov pc, lr\n\t"
    );
}
static __inline void sti()
{
    asm volatile(
      "mrs r0, cpsr\n\t"
      "orr r0, r0, #0x80\n\t"
      "msr cpsr_c, r0\n\t"
      "mov pc, lr\n\t"
    );
}
/*
static __inline void
cpu_idle()
{
    __asm__ __volatile__("hlt");
}

#define save_flags_cli(flags)   \
    do {                        \
        __asm__ __volatile__(   \
                "pushfl\n\t"    \
                "popl %0\n\t"   \
                "cli\n\t"       \
                :"=g"(flags): );\
    } while(0)

#define restore_flags(flags)    \
    do {                        \
        __asm__ __volatile__(   \
                "pushl %0\n\t"  \
                "popfl\n\t"     \
                ::"g"(flags)    \
                :"memory" );    \
    } while(0)

static __inline void
invlpg(uint32_t addr)
{
    __asm__ __volatile__("invlpg %0" : : "m" (*(char *)addr) : "memory");
}

static __inline void
invltlb(void)
{
    uint32_t temp;
    __asm__ __volatile__("movl %%cr3, %0; movl %0, %%cr3" : "=r" (temp)
            : : "memory");
}
*/
#define PAGE_SHIFT  12
#define PGDR_SHIFT  20
#define PAGE_SIZE   (1<<PAGE_SHIFT)
#define PAGE_MASK   (PAGE_SIZE-1)
#define PAGE_TRUNCATE(x)  ((x)&(~PAGE_MASK))
#define PAGE_ROUNDUP(x)   (((x)+PAGE_MASK)&(~PAGE_MASK))

#define PTE_V   0x001 /* Valid */
#define PTE_W   0x002 /* Read/Write */
#define PTE_U   0x004 /* User/Supervisor */
#define PTE_A   0x020 /* Accessed */
#define PTE_M   0x040 /* Dirty */

#endif /*_CPU_H*/
