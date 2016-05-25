/**
 * vim: filetype=c:fenc=utf-8:ts=4:et:sw=4:sts=4
 *
 * Copyright (C) 2008, 2013 Hong MingJian<hongmingjian@gmail.com>
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
#include <string.h>
#include "kernel.h"
#include "frame.h"
#include "bitmap.h"

unsigned char task_run(unsigned char TID);
unsigned char task_get_id();
void task_delete();
unsigned char task_create(unsigned char rank, unsigned int task_func);
void task_idle(void);

struct tcb1 task_global;
void task_init()
{
    int i;
    for(i = 0; i < 256; i++)
    {
      task_info[i].rank = 0;
      task_info[i].status = 0;
      task_info[i].task_func = 0;
      task_info[i].TRID = 256;
    }
		unsigned char rank = MAX_rank ;
		unsigned int task_func = (unsigned int)task_idle;
		unsigned char TID = task_create( rank , task_func);
		task_run(TID);

		//初始化task_global
		task_global.schedule_lock = false;
		task_global.tid = TID;
		task_global.kstack = (unsigned int) &task_table[TID];
}

void task_idle(void)
{
  while(1)
  {
    uart_puts("init task success!\r\n");
    sleep(100);
  }
}

unsigned char task_create(unsigned char rank, unsigned int task_func)
{
		/*1.检查参数合法性*/
		if( rank > MAX_rank + 1)
		{
				return 0;
		}

		/*2.设置TASK_INFO*/
		unsigned char TID = task_get_id();

		TASK_INFO task_info_this;
		task_info_this.rank = rank;
		task_info_this.status = DEAD ;
		task_info_this.task_func = task_func ;
		task_info[TID] = task_info_this;
		//task_info.TRID 到 task_run 中设置

		/*3.设置tasktable*/
    task_table[TID].r0 = 0;
    task_table[TID].r1 = 0;
    task_table[TID].r2 = 0;
    task_table[TID].r3 = 0;
    task_table[TID].r4  = 0;
    task_table[TID].r5 = 0;
    task_table[TID].r6 = 0;
    task_table[TID].r7 = 0;
    task_table[TID].r8 = 0;
    task_table[TID].r9 = 0;
    task_table[TID].r10 = 0;
    task_table[TID].r11 = 0;
    task_table[TID].r12 = 0;
		task_table[TID].sp = (unsigned int)task_stack[TID] + 1024;
		task_table[TID].lr = (unsigned int)task_delete;
		task_table[TID].pc =(unsigned int) task_func;
    task_table[TID].cpsr = 0x53;//禁止fiq , svc模式
		task_table[TID].spsr = 0x53;//禁止fiq , svc模式
		return TID;
}

unsigned char task_get_id()
{
		int id = 0;
		for(id = 0; id < 256; id++)
		{
			if( (task_info[id].rank == 0) && (task_info[id].status == 0) && (task_info[id].task_func == 0 ) && (task_info[id].TRID == 256 ) )
			{
				return id;
			}
		}
		return 0;
}

unsigned char task_run(unsigned char TID)
{
		/*1.参数验证, run 的进程状态必须是DEAD,其他状态的进程不可以被run*/
		if(task_info[TID].status != DEAD)
		{
				return 0;
		}

		/*2.1 申请加入task_ready链表, 返回0失败*/
		task_info[TID].TRID = TID;//ll_add_by_order(task_ready, TID, task_info[TID].rank);

    task_info[TID].status = READY;
		return task_info[TID].TRID;
}

void  task_schedule(void)
{
		unsigned char current_TID , current_TRID;
		current_TID = current_TRID = task_info[task_global.tid].TRID;										//获取当前TRID

    int i;
    for(i = 0; i < 17; i++)
    {
      unsigned int _temp = task_info[(current_TRID+i+1)%256].TRID;
      if(17 != _temp)
      {
        current_TID = _temp;
        break;
      }
    }

		task_global.tid = current_TID;			//设置当前TID													//
		task_global.kstack = (unsigned int) &task_table[task_global.tid];	//设置当前tasktable

		return ;
}

void task_delete()
{
	unsigned char TID = task_global.tid;
	task_info[TID].TRID = 0;
	task_info[TID].rank = 0 ;
	task_info[TID].status = 0;
	task_info[TID].task_func = 0;

	task_table[TID].r0 = 0;
	task_table[TID].r1 = 0;
	task_table[TID].r2 = 0;
	task_table[TID].r3 = 0;
	task_table[TID].r4  = 0;
	task_table[TID].r5 = 0;
	task_table[TID].r6 = 0;
	task_table[TID].r7 = 0;
	task_table[TID].r8 = 0;
	task_table[TID].r9 = 0;
	task_table[TID].r10 = 0;
	task_table[TID].r11 = 0;
	task_table[TID].r12 = 0;
	task_table[TID].sp = 0;
	task_table[TID].lr = 0;
	task_table[TID].pc = 0;
	task_table[TID].spsr = 0;
}

void task_lock_schedule(void)
{
	task_global.schedule_lock = true;
}

void task_unlock_schedule(void)
{
	task_global.schedule_lock = false;
}



int g_resched;
struct tcb *g_task_head;
struct tcb *g_task_running;
struct tcb *task0;
struct tcb *g_task_own_fpu;

/**
 * CPU调度器函数，这里只实现了轮转调度算法
 *
 * 注意：该函数的执行不能被中断
 */
void schedule()
{
    struct tcb *select = g_task_running;
    do {
        select = select->next;
        if(select == NULL)
            select = g_task_head;
        if(select == g_task_running)
            break;
        if((select->tid != 0) &&
           (select->state == TASK_STATE_READY))
            break;
    } while(1);

    if(select == g_task_running) {
        if(select->state == TASK_STATE_READY)
            return;
        select = task0;
    }

    //printk("0x%d -> 0x%d\r\n", (g_task_running == NULL) ? -1 : g_task_running->tid, select->tid);

    g_resched = 0;
  //  switch_to(select);
}

/**
 * 把当前线程切换为等待状态，等待在*head队列中
 *
 * 注意：该函数的执行不能被中断
 */
void sleep_on(struct wait_queue **head)
{
    struct wait_queue wait;

    wait.tsk = g_task_running;
    wait.next = *head;
    *head = &wait;

    g_task_running->state = TASK_STATE_WAITING;
    schedule();

    if(*head == &wait)
        *head = wait.next;
    else {
        struct wait_queue *p, *q;
        p = *head;
        do {
            q = p;
            p = p->next;
            if(p == &wait) {
                q->next = p->next;
                break;
            }
        } while(p != NULL);
    }
}

/**
 * 唤醒n个等待在*head队列中的线程。
 * 如果n<0，唤醒队列中的所有线程
 *
 * 注意：该函数的执行不能被中断
 */
void wake_up(struct wait_queue **head, int n)
{
    struct wait_queue *p;

    for(p = *head; (p!=NULL) && n; p = p->next, n--)
        p->tsk->state = TASK_STATE_READY;
}

static
void add_task(struct tcb *tsk)
{
    if(g_task_head == NULL)
        g_task_head = tsk;
    else {
        struct tcb *p, *q;
        p = g_task_head;
        do {
            q = p;
            p = p->next;
        } while(p != NULL);
        q->next = tsk;
    }
}

static
void remove_task(struct tcb *tsk)
{
    if(g_task_head != NULL) {
        if(tsk == g_task_head) {
            g_task_head = g_task_head->next;
        } else {
            struct tcb *p, *q;
            p = g_task_head;
            do {
                q = p;
                p = p->next;
                if(p == tsk)
                    break;
            } while(p != NULL);

            if(p == tsk)
                q->next = p->next;
        }
    }
}

static
struct tcb* get_task(int tid)
{
    struct tcb *tsk;

    tsk = g_task_head;
    while(tsk != NULL) {
        if(tsk->tid == tid)
            break;
        tsk = tsk->next;
    }

    return tsk;
}

/**
 * 系统调用task_create的执行函数
 *
 * 创建一个新的线程，该线程执行func函数，并向新线程传递参数pv
 */
struct tcb *sys_task_create(void *tos,
                            void (*func)(void *pv), void *pv)
{
    static int tid = 0;
    struct tcb *new;
    char *p;
    uint32_t flags;
    uint32_t ustack=(uint32_t)tos;

    if(ustack & 3)
        return NULL;

    p = (char *)kmalloc(PAGE_SIZE);
    if(p == NULL)
        return NULL;

    new = (struct tcb *)p;

    memset(new, 0, sizeof(struct tcb));

    new->kstack = (uint32_t)(p+PAGE_SIZE);
    new->tid = tid++;
    new->state = TASK_STATE_READY;
    new->timeslice = TASK_TIMESLICE_DEFAULT;
    new->wq_exit = NULL;
    new->next = NULL;
    new->signature = TASK_SIGNATURE;

    if(ustack != 0) {
        STACK_PUSH(ustack, pv);
        STACK_PUSH(ustack, 0);
    } else {
        STACK_PUSH(new->kstack, pv);
        STACK_PUSH(new->kstack, 0);
    }

    //INIT_TASK_CONTEXT(ustack, new->kstack, func);

    //save_flags_cli(flags);
    add_task(new);
    //restore_flags(flags);

    return new;
}

/**
 * 系统调用task_exit的执行函数
 *
 * 结束当前线程，code_exit是它的退出代码
 */
void sys_task_exit(int code_exit)
{
    uint32_t flags;

    //save_flags_cli(flags);

    wake_up(&g_task_running->wq_exit, -1);

    g_task_running->code_exit = code_exit;
    g_task_running->state = TASK_STATE_ZOMBIE;

    if(g_task_own_fpu == g_task_running)
        g_task_own_fpu = NULL;

    schedule();
}

/**
 * 系统调用task_wait的执行函数
 *
 * 当前线程等待线程tid结束执行。
 * 如果pcode_exit不是NULL，用于保存线程tid的退出代码
 */
int sys_task_wait(int tid, int *pcode_exit)
{
    uint32_t flags;
    struct tcb *tsk;

    if(g_task_running == NULL)
        return -1;

    //save_flags_cli(flags);

    if((tsk = get_task(tid)) == NULL) {
        //restore_flags(flags);
        return -1;
    }

    if(tsk->state != TASK_STATE_ZOMBIE)
        sleep_on(&tsk->wq_exit);

    if(pcode_exit != NULL)
        *pcode_exit= tsk->code_exit;

    if(tsk->wq_exit == NULL) {
        remove_task(tsk);
        //printk("%d: Task %d reaped\r\n", sys_task_getid(), tsk->tid);
        //restore_flags(flags);

        kfree(tsk);
        return 0;
    }

    //restore_flags(flags);
    return 0;
}

/**
 * 系统调用task_getid的执行函数
 *
 * 获取当前线程的tid
 */
int sys_task_getid()
{
    return (g_task_running==NULL)?-1:g_task_running->tid;
}

/**
 * 系统调用task_yield的执行函数
 *
 * 当前线程主动放弃CPU，让调度器调度其他线程运行
 */
void sys_task_yield()
{
    uint32_t flags;
    //save_flags_cli(flags);
    schedule();
    //restore_flags(flags);
}

/**
 * 初始化多线程子系统
 */
void init_task()
{
    g_resched = 0;
    g_task_running = NULL;
    g_task_head = NULL;
    g_task_own_fpu = NULL;

    /*
     * 创建线程task0，即系统空闲线程
     */
    task0 = sys_task_create(NULL, NULL/*task0执行的函数将由run_as_task0填充*/, NULL);
}
