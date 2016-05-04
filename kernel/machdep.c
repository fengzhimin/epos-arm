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
    memset(pgdir, 0, 1<<14);

    /*分配20张小页表 并且将填充页目录*/
    for(i = 0; i < NR_KERN_PAGETABLE; i++)
    {
        pgdir[i] = pgdir[i+(KERNBASE>>PGDR_SHIFT)] = (physfree&0xFFFFFC00)|PTE_V;
        memset((void *)physfree, 0, (1<<10));
        physfree += (1<<10);
    }

    /*设置恒等隐射，填充小页表 隐射物理地址为[0, &_end] 虚拟地址[0, &_end] [KERNBASE, R(&_end)]*/
    for(i = 0; i < (PAGE_ROUNDUP((unsigned int)(&_end)>>12)); i++)
    {
        pte[i] = ((i<<12)&0xFFFFF000)|0xFF|PTE_W;
    }

    /*设置IO端口隐射，物理地址为[20000000, 20220000] 虚拟地址[0xC1000000 0xC1220000]
    从第17张小页表开始隐射IO设备地址 第17张小页表的第一项index是0x1000
    */
    for(i = 0; i < 0x220; i++)
        pte[0x1000+i] = ((0x20000000+(i<<12))&0xFFFFF000)|0xFF|PTE_W;

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
             : "r" (pgdir)//(pgdir+0xC0000000)
             : "r0"
     );

    return physfree;
}

/**
 * 初始化物理内存
*/
static void init_ram(uint32_t physfree)
{
    g_ram_zone[0] = physfree;
    g_ram_zone[1] = 0x1FFFFFFF;
    g_ram_zone[2] = 0;
    g_ram_zone[3] = 0;
}



/**
 * 机器相关（Machine Dependent）的初始化
 */
static void md_startup(uint32_t mbi, uint32_t physfree)
{
    physfree=init_paging(physfree);

    init_ram(physfree);


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
    unsigned int * page_base = (unsigned int *)(end + 0xC0000000);

    /*
     * 机器相关（Machine Dependent）的初始化
     */
    md_startup(mbi, end);
    unsigned int physfree = end;
    /*
     * 分页已经打开，切换到虚拟地址运行
     */
     /* 重定位内核*/
     	asm volatile(
     		"mov r0, pc\n\t"
     		"add r0, r0, #0xC0000000\n\t"
     		"add r13, r13, #0xC0000000\n\t"
        "add r14, r14, #0xC0000000\n\t"
     		"add pc, r0, #0xC\n\t"
     	);

      init_arm_timer(Kernel_1Hz);
      cli();
      uart_init();
      char *_ch1 = "0x00000000\r\n";
      HexToString(physfree, _ch1);
      uart_puts(_ch1);
      sleep(2000);

    /* 清除恒等隐射
    int i;
    for(i= 0; i < NR_KERN_PAGETABLE; i++)
      page_base[i] = 0;
*/
    while(1)
    {
      int i;
      for(i = 0; i < 20; i++)
      {
        HexToString(page_base[i], _ch1);
        uart_puts(_ch1);
      }

      sleep(500);
      sti();
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
