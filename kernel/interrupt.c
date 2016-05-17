#include "uart.h"
#include "timer.h"
#include "machdep.h"
#include "string.h"

extern arm_timer_t *ArmTimer;

void common_undef_handler(void)
{
	sleep(500);
	char *_ch = "undefine interrupts\r\n";
	uart_puts(_ch);
}

void common_swi_handler(void)
{
	sleep(500);
	char *_ch = "swi interrupts\r\n";
	uart_puts(_ch);
}

void common_pref_handler(uint32_t vaddr, uint32_t code)
{
	char *_ch = "prefetch interrupts\r\n";
	uart_puts(_ch);

	char *_temp = "0000000000\r\n";
	HexToString(vaddr, _temp);
	uart_puts("page fault address:");
	uart_puts(_temp);
	HexToString(code, _temp);
	uart_puts("page fault code:");
	uart_puts(_temp);

	sleep(5000);
}

void common_Dabt_handler(uint32_t vaddr, uint32_t code)
{
	char *_ch = "data abort interrupts\r\n";
	uart_puts(_ch);
	char *_temp = "0000000000\r\n";
	HexToString(vaddr, _temp);
	uart_puts("page fault address:");
	uart_puts(_temp);
	HexToString(code, _temp);
	uart_puts("page fault code:");
	uart_puts(_temp);
	sleep(500);
}

void common_res_handler(void)
{
	sleep(500);
	char *_ch = "reserved interrupts\r\n";
	uart_puts(_ch);
}

void common_irq_handler(struct context *cf)
{
	//ArmTimer->Control = ArmTimer->Control | ARMTIMER_CTRL_INT_DISABLE;
	ArmTimer->IRQClear = 0x0;

	uart_puts("timer interrupts\r\n");
	printfContext(cf);
	uart_puts("timer interrupts end!\r\n");
	//ArmTimer->Control = ArmTimer->Control | ARMTIMER_CTRL_INT_ENABLE;
}


void common_fiq_handler(void)
{
	char *_ch = "fiq interrupts\r\n";
	uart_puts(_ch);
}
