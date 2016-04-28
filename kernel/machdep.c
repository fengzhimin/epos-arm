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
#include <stddef.h>
#include <syscall-nr.h>
#include <ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include "uart.h"
#include "timer.h"

#include "kernel.h"

#define __CONCAT1(x,y)  x ## y
#define __CONCAT(x,y)   __CONCAT1(x,y)
#define __STRING(x)     #x              /* stringify without expanding x */
#define __XSTRING(x)    __STRING(x)     /* expand x, then stringify */

extern unsigned int _end;
extern void _enable_interrupts();

/*计算机启动时，自1970-01-01 00:00:00 +0000 (UTC)以来的秒数*/
time_t g_startup_time;


/**
 * 初始化中断控制器
 */
static void init_i8259(uint8_t idt_offset)
{

}

/**
 * 初始化定时器
 */
static void init_i8253(uint32_t freq)
{

}

/**
 * 让中断控制器打开某个中断
 */
void enable_irq(uint32_t irq)
{

}

/**
 * 让中断控制器关闭某个中断
 */
void disable_irq(uint32_t irq)
{

}

/**
 * 系统调用putchar的执行函数
 *
 * 往屏幕上的当前光标位置打印一个字符，相应地移动光标的位置
 */
int sys_putchar(int c)
{
    return c;
}

/**
 * 初始化分页子系统
 */
static uint32_t init_paging(uint32_t physfree)
{
    uint32_t i;
    uint32_t *pgdir, *pte;

    /*
     * 分配页目录
     */
    pgdir=(uint32_t *)physfree;
    physfree += 1<<14;//physfree += PAGE_SIZE;
    pte = (uint32_t *)physfree;
    //memset(pgdir, 0, 1<<14);

    /*
     * 分配小页表，并填充页目录

    for(i = 0; i < NR_KERN_PAGETABLE; i++) {
        pgdir[i                       ]=
        pgdir[i+(KERNBASE>>PGDR_SHIFT)]=physfree|PTE_V|PTE_W;
        memset((void *)physfree, 0, PAGE_SIZE);
        physfree+=PAGE_SIZE;
    }
    */
    /*分配20张小页表 并且将填充页目录*/
    for(i = 0; i < 20; i++)
    {
        pgdir[i] = pgdir[i+(0xC0000000>>20)] = (physfree&0xFFFFFC00)|1;
        //memset((void *)physfree, 0, 1<<10);
        physfree += 1<<10;
    }

    /*设置恒等隐射，填充小页表 隐射物理地址为[0, 0x800000] 虚拟地址[0, 0x800000] [0xC0000000, 0xC0800000]*/
    for(i = 0; i < 0x1100; i++)
    {
        pte[i] = ((i<<12)&0xFFFFF000)|0xFF|2;
    }

    /*设置IO端口隐射，物理地址为[20000000, 20220000]*/
    for(i = 0; i < 0x220; i++)
        pte[0x1100+i] = ((0x20000000+(i<<12))&0xFFFFF000)|0xFF|2;
    /*
     * 映射映射虚拟地址[0, R(&end)]和[KERNBASE, &end]到物理地址[0, R(&end)]

    pte=(uint32_t *)(PAGE_TRUNCATE(pgdir[0]));
    for(i = 0; i < (uint32_t)(pgdir); i+=PAGE_SIZE)
        pte[i>>PAGE_SHIFT]=(i)|PTE_V|PTE_W;
    */
    /*
     * 映射页目录及页表

    pgdir[(KERNBASE>>PGDR_SHIFT)-1]=(uint32_t)(pgdir)|PTE_V|PTE_W;
     */

    /*
     * 打开分页
     */
     asm(
             "mcr p15,0,%0,c2,c0,0\n"
             "mvn r0,#0\n"
             "mcr p15,0,r0,c3,c0,0\n"
             "mov r0,#0x1\n"
             "mcr p15,0,r0,c1,c0,0\n"
             "mov r0,r0\n"
             "mov r0,r0\n"
             "mov r0,r0\n"
             :
             : "r" (pgdir)
             : "r0"
     );

    return physfree;
}

/**
 * 初始化物理内存

static void init_ram(multiboot_memory_map_t *mmap,
        uint32_t size,
        uint32_t physfree)
{
    int n = 0;

    for (; size;
            size -= (mmap->size+sizeof(mmap->size)),
            mmap = (multiboot_memory_map_t *) ((uint32_t)mmap +
                                               mmap->size +
                                               sizeof (mmap->size))) {
        if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
            g_ram_zone[n  ] = PAGE_TRUNCATE(mmap->addr&0xffffffff);
            g_ram_zone[n+1] = PAGE_TRUNCATE(g_ram_zone[n]+(mmap->len&0xffffffff));

            //扣除内核所占的物理内存
            if((physfree >  g_ram_zone[n  ]) &&
               (physfree <= g_ram_zone[n+1]))
                g_ram_zone[n]=physfree;

            //为8086模式保留0-4KiB的物理内存
            if((PAGE_SIZE >  g_ram_zone[n  ]) &&
               (PAGE_SIZE <= g_ram_zone[n+1]))
                g_ram_zone[n]=PAGE_SIZE;

            if(g_ram_zone[n+1] >= g_ram_zone[n] + PAGE_SIZE) {
                n += 2;
                if(n + 2 >= RAM_ZONE_LEN)
                    break;
            }
        }
    }

    g_ram_zone[n  ] = 0;
    g_ram_zone[n+1] = 0;
}
 */


/**
 * 机器相关（Machine Dependent）的初始化
 */
static void md_startup(uint32_t mbi, uint32_t physfree)
{
    physfree=init_paging(physfree);
/*
    init_ram((void *)(((multiboot_info_t *)mbi)->mmap_addr),
             ((multiboot_info_t *)mbi)->mmap_length,
             physfree);
*/

    //init_i8259(ICU_IDT_OFFSET);
    //init_i8253(HZ);
}

/**
 * 这个函数是内核的C语言入口，被entry.S调用
 */
void cstart(uint32_t magic, uint32_t mbi)
{
    //uint32_t end = PAGE_ROUNDUP(R((uint32_t)(&_end)))+(0x1<<14);
    unsigned int end = (unsigned int)(&_end+(0x1<<14))&0xffffc000;

    /*
     * 机器相关（Machine Dependent）的初始化
     */
    md_startup(mbi, end);

    /*
     * 分页已经打开，切换到虚拟地址运行
     */
     /* 重定位内核*/
     	asm volatile(
     		"mov r0, pc\n\t"
     		"add r0, r0, #0xc0000000\n\t"
     		"add lr, lr, #0xC0000000\n\t"
     		"add sp, sp, #0xC0000000\n\t"
     		"add pc, r0, #0xc\n\t"
     	);

    init_arm_timer(Kernel_1Hz);
	  _enable_interrupts();
    uart_init();
    char *_ch1 = "init epos kernel success!\r\n";
    int i;
    while(1)
    {
      uart_puts(_ch1);
      sleep(500);
    }

    /*
     * 内核已经被重定位到链接地址，取消恒等映射

    for(i = 0; i < NR_KERN_PAGETABLE; i++)
        PTD[i] = 0;
     */

    /*
     * 取消了1MiB以内、除文本显存以外的虚拟内存映射

    for(i = 0; i < 0x100000; i+=PAGE_SIZE)
        if(i != 0xB8000)
            *vtopte(i+KERNBASE)=0;
 */

    /*
     * 机器无关（Machine Independent）的初始化
     */
    mi_startup();
}
