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
	uart_putc(c);
    return c;
}

void task_idle0(void)
{
  printk("first task create success!\r\n");
  while(1);
}

void task_idle1(void)
{
  printk("second task create success!\r\n");
  while(1);
}

void task_idle2(void)
{
  printk("third task create success!\r\n");
  while(1);
}

/**
 * 初始化分页子系统
 */
static uint32_t init_paging(uint32_t physfree)
{
    uint32_t i;
    uint32_t *pgdir, *pte;

#if 1
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
#else

#define L1_TABLE_SIZE   0x4000          /* 16K */
#define L2_TABLE_SIZE_REAL 0x400 /* 1K */
#define L2_PTE_NUM_TOTAL (L2_TABLE_SIZE_REAL / 4)

#define L1_TYPE_C       0x01            /* Coarse L2 */
#define L1_C_DOM(x) ((x) << 5) /* domain */
#define L1_C_ADDR_MASK 0xfffffc00 /* phys address of L2 Table */

#define L2_TYPE_S       0x02            /* Small Page */
#define L2_B            0x00000004      /* Bufferable page */
#define L2_C            0x00000008      /* Cacheable page */
#define L2_AP0(x)       (((x) & 0x3) << 4)      /* access permissions (sp 0) */
#define L2_AP1(x)       (((x) & 0x3) << 6)      /* access permissions (sp 1) */
#define L2_AP2(x)       (((x) & 0x3) << 8)      /* access permissions (sp 2) */
#define L2_AP3(x)       (((x) & 0x3) << 10)     /* access permissions (sp 3) */
#define L2_S_TEX(x)     (((x) & 0x7) << 6)
#define L2_SHARED       (1 << 10)
#define L2_APX          (1 << 9)


    /*
     * 分配页目录
     */
    pgdir=(uint32_t *)physfree;
    physfree += L1_TABLE_SIZE;
    memset(pgdir, 0, L1_TABLE_SIZE);

    /*分配4张小页表 并填充到页目录*/
    uint32_t *xxx = (uint32_t *)physfree;
    for(i = 0; i < 4; i++)
    {
        pgdir[0xBFC+i] = (physfree)|L1_C_DOM(0)|L1_TYPE_C;
        memset((void *)physfree, 0, L2_TABLE_SIZE_REAL);
        physfree += L2_TABLE_SIZE_REAL;
    }

    /*分配NR_KERN_PAGETABLE张小页表 并填充页目录*/
    pte = (uint32_t *)physfree;
    for(i = 0; i < NR_KERN_PAGETABLE; i++)
    {
        pgdir[i] = pgdir[i+(KERNBASE>>PGDR_SHIFT)] = (physfree)|L1_C_DOM(0)|L1_TYPE_C;

		if((i & 3) == 0)
			xxx[3*L2_PTE_NUM_TOTAL+(i>>2)] = (physfree)|L2_S_TEX(0)|L2_AP0(1)|L2_C|L2_B|L2_TYPE_S;

        memset((void *)physfree, 0, L2_TABLE_SIZE_REAL);
        physfree += L2_TABLE_SIZE_REAL;
    }
    xxx[3*L2_PTE_NUM_TOTAL-1] = ((uint32_t)xxx)|L2_S_TEX(0)|L2_AP0(1)|L2_C|L2_B|L2_TYPE_S;


	/*设置恒等映射，填充小页表 映射物理地址为[0, 0x4000] 虚拟地址[0, 0x4000] [KERNBASE, KERNBASE+0x4000]*/
    for(i = 0; i < 0x4000; i+=PAGE_SIZE)
    {
      pte[i>>PAGE_SHIFT] = i|L2_S_TEX(0)|L2_AP0(1)|L2_C|L2_B|L2_TYPE_S;
    }

    /*把页目录映射到[0x4000, 0x8000]和[KERNBASE+0x4000, KERNBASE+0x8000]*/
    for(i = 0x4000; i < 0x8000; i+=PAGE_SIZE)
    {
      pte[i>>PAGE_SHIFT] = (((uint32_t)pgdir)+i-0x4000)|L2_S_TEX(0)|L2_AP0(1)|L2_C|L2_B|L2_TYPE_S;
    }

    /*设置恒等映射，填充小页表 映射物理地址为[0x8000, R(_end)] 虚拟地址[0x8000, R(_end)] [KERNBASE+0x8000, _end]*/
    for(i = 0x8000; i < (uint32_t)pgdir; i+=PAGE_SIZE)
    {
      pte[i>>PAGE_SHIFT] = i|L2_S_TEX(0)|L2_AP0(1)|L2_C|L2_B|L2_TYPE_S;
    }
#endif


    /*
     * 打开分页
     */
     asm(
             "mcr p15,0,%0,c2,c0,0\n\t"
             "mvn r0, #0\n\t"
             "mcr p15,0,r0,c3,c0,0\n\t"
			 "mrc p15,0,r0,c1,c0,0\n\t"
             "orr r0, r0, #1\n\t"
             "mcr p15,0,r0,c1,c0,0\n\t"
             "mov r0,r0\n\t"
             "mov r0,r0\n\t"
             :
             : "r" (pgdir)
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
    unsigned int end = (((uint32_t)(&_end))+(0x4000-1))&(~(0x4000-1));

    /*
     * 机器相关（Machine Dependent）的初始化
     */
    md_startup(mbi, R(end));

	 /*Physical addresses range from 0x20000000 to 0x20FFFFFF for peripherals*/
    //page_map(0xC1000000, 0x20000000, 4096, L2_S_TEX(0)|L2_AP0(1)|L2_TYPE_S);

    uart_init();
		uint32_t vaddr, code;
		vaddr = 800;
		code = 5;
		printk("Welcome to ARM-EPOS\r\n");
		printk("PF:0x%d(0x%d)", vaddr, code);
    printk("Copyright (C) 2005-2015 MingJian Hong<hongmingjian@gmail.com>\r\n");
    printk("Author: fengzhimin \r\n");
    printk("All rights reserved.\r\n\r\n");
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
    /* 清除恒等隐射
    for(i= 1; i < NR_KERN_PAGETABLE; i++)
      PTD[i] = 0;*/
    while(1);

    /*
     * 机器无关（Machine Independent）的初始化
     */
    mi_startup();

}
