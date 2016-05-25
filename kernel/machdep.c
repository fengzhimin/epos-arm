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
extern void cli();
extern void sti();

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

void task_idle0(void)
{
  while(1)
  {
    int i;
    for(i = 0; i < 10; i++)
    {
      uart_puts("Hello first task create success!");
      uart_putc(i+'0');
      uart_puts("\r\n");
    }
  }
}

void task_idle1(void)
{
  int i;
  while(1)
  {
    for(i = 0; i < 10; i++)
    {
      uart_puts("Hello second task create success!");
      uart_putc(i+'0');
      uart_puts("\r\n");
    }
  }
}

void task_idle2(void)
{
  int i;
  while(1)
  {
    for(i = 0; i < 10; i++)
    {
      uart_puts("Hello third task create success!");
      uart_putc(i+'0');
      uart_puts("\r\n");
    }
  }
}

void task_idle3(void)
{
  int i;
  for(i = 0; i < 10; i++)
  {
    uart_puts("Hello fourth task create success!");
    uart_putc(i+'0');
    uart_puts("\r\n");
  }
  task_delete();
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
        pgdir[i] = pgdir[i+(KERNBASE>>PGDR_SHIFT)] = (physfree)|PTE_V;
        memset((void *)physfree, 0, (1<<10));
        physfree += (1<<10);
    }

    /*单独给中断向量表分配一张小页表
    pgdir[0xFFF] = physfree|PTE_V;
    memset((void *)physfree, 0, (1<<10));
    physfree += (1<<10);*/

    /*设置恒等隐射，填充小页表 隐射物理地址为[0, &_end] 虚拟地址[0, &_end] [KERNBASE, R(&_end)]*/
    for(i = 0; i < (PAGE_ROUNDUP(physfree)>>12); i++)
    {
      pte[i] = (i<<12)|PTE_W|PTE_V;
    }

    /*设置IO端口隐射，物理地址为[20000000, 20220000] 虚拟地址[0xC1000000 0xC1220000]
    从第17张小页表开始隐射IO设备地址 第17张小页表的第一项index是0x1000
    */
    for(i = 0; i < 0x220; i++)
        pte[0x1000+i] = ((0x20000000+(i<<12))&0xFFFFF000)|PTE_V|PTE_W;

    /*隐射中断向量表，物理地址为[0x00000000, 0x00001000] 虚拟地址[0xFFFF0000, 0xFFFF1000]*/
    pte[20*256+0xF0] = 0x00000000|PTE_V|PTE_W;   /*20*2^8+0xF0 为0xFFFF0000的页表项*/

    /*
        for(i = 0 ; i < 4; i++) {
          pgdir[0xbff-i]=(((uint32_t)pgdir)+i*(1<<12))|PTE_V;
        }
    */

    /*
    * 开启中断向量表高地址隐射(0xFFFFF000~0xFFFFF001C)

    asm(
            "mvn r0,#0x0\n"
            "mcr p15, 0, r0, c12, c0, 0\n"
    );*/

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
    unsigned int end = (unsigned int)(((&_end+(0x1<<14))))&0xffffc000;
    PTD = (unsigned int *)(end);
    PT = (uint32_t*)((uint32_t)(PTD)+(0x1<<14));

    /*
     * 机器相关（Machine Dependent）的初始化
     */
    md_startup(mbi, end-0xC0000000);

    uart_init();
    char *_ch1 = "0x00000000\r\n";
    task_init();
  	unsigned char rank = MAX_rank ;
  	unsigned int task_func = (unsigned int)task_idle0;
  	unsigned char TID = task_create( rank , task_func);
  	task_run(TID);
  	task_func = (unsigned int)task_idle1;
  	TID = task_create( rank , task_func);
  	task_run(TID);
  	task_func = (unsigned int)task_idle2;
  	TID = task_create( rank , task_func);
  	task_run(TID);
    task_func = (unsigned int)task_idle3;
  	TID = task_create( rank , task_func);
  	task_run(TID);
    init_arm_timer(Kernel_1Hz);
    cli();

    /*
     * 分页已经打开，切换到虚拟地址运行
     */
     /*重定位内核*/
     asm (
       "add sp, sp, #0xC0000000\n\t"
       "add r11, r11, #0xC0000000\n\t"
     );

     int i;

    /* 清除恒等隐射 */
    for(i= 1; i < NR_KERN_PAGETABLE; i++)
      PTD[i] = 0;

    while(1)
    {
      for(i = 0; i < 20; i++)
      {
        HexToString(PTD[i], _ch1);
        uart_puts(_ch1);
      }

      for(i = 0; i < 20; i++)
      {
        HexToString(PTD[i+0xC00], _ch1);
        uart_puts(_ch1);
      }
      /*
      asm (
        "mov pc, #0xB0000000\n\t"
      );
      uint32_t *_pagefaultAddr = (uint32_t *)0xB0000000;
      *_pagefaultAddr = 0x1;
      //uint32_t _pagefautData = (uint32_t)(*_pagefaultAddr);

      sleep(500);
      //sti();
      */
    }


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
