#include "uart.h"
#include "timer.h"

extern arm_timer_t *ArmTimer;

void common_irq_handler(void)
{
	//sleep(500);
	ArmTimer->IRQClear = 0x0;
	char *_ch = "epos timer interrupts\r\n";
	uart_puts(_ch);
}
