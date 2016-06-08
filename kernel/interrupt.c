#include "uart.h"
#include "timer.h"
#include "machdep.h"
#include "string.h"
#include "kernel.h"

extern arm_timer_t *ArmTimer;

void common_undef_handler(void)
{
	sleep(500);
	char *_ch = "undefine interrupts\r\n";
	printk(_ch);
}

void common_swi_handler(void)
{
	sleep(500);
	char *_ch = "swi interrupts\r\n";
	printk(_ch);
}

void common_pref_handler(uint32_t vaddr, uint32_t code)
{
	char *_ch = "prefetch interrupts\r\n";
	printk(_ch);

	char *_temp = "0000000000\r\n";
	HexToString(vaddr, _temp);
	printk("page fault address:");
	printk(_temp);
	HexToString(code, _temp);
	printk("page fault code:");
	printk(_temp);

	sleep(5000);
}

void common_Dabt_handler(uint32_t vaddr, uint32_t code)
{
	char *_ch = "data abort interrupts\r\n";
	printk(_ch);
	char *_temp = "0000000000\r\n";
	HexToString(vaddr, _temp);
	printk("page fault address:");
	printk(_temp);
	HexToString(code, _temp);
	printk("page fault code:");
	printk(_temp);
	if(code == 0x805)
		do_page_fault(NULL, vaddr, VM_PROT_WRITE);
	else
		do_page_fault(NULL, vaddr, VM_PROT_READ);

	sleep(500);
}

void common_res_handler(void)
{
	sleep(500);
	char *_ch = "reserved interrupts\r\n";
	printk(_ch);
}

void common_irq_handler(struct context *cf)
{
	//ArmTimer->Control = ArmTimer->Control | ARMTIMER_CTRL_INT_DISABLE;
	ArmTimer->IRQClear = 0x0;

	printk("timer interrupts\r\n");
	//printfContext(cf);
	//printk("timer interrupts end!\r\n");
	//ArmTimer->Control = ArmTimer->Control | ARMTIMER_CTRL_INT_ENABLE;
}


void common_fiq_handler(void)
{
	char *_ch = "fiq interrupts\r\n";
	printk(_ch);
}
