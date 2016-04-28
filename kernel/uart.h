#ifndef __UART_H__
#define __UART_H__

#define AUX_ENABLES     0xC1315004
#define AUX_MU_IO_REG   0xC1315040
#define AUX_MU_IER_REG  0xC1315044
#define AUX_MU_IIR_REG  0xC1315048
#define AUX_MU_LCR_REG  0xC131504C
#define AUX_MU_MCR_REG  0xC1315050
#define AUX_MU_LSR_REG  0xC1315054
#define AUX_MU_MSR_REG  0xC1315058
#define AUX_MU_SCRATCH  0xC131505C
#define AUX_MU_CNTL_REG 0xC1315060
#define AUX_MU_STAT_REG 0xC1315064
#define AUX_MU_BAUD_REG 0xC1315068
#define GPFSEL1    0xC1300004UL
#define GPPUD      0xC1300094UL
#define GPPUDCLK0  0xC1300098UL
#define IRQ_DISABLE1 0xC110B21C
#define IRQ_ENABLE1 0xC110B210

void mmio_write(unsigned int reg, unsigned int data);
unsigned int mmio_read(unsigned int reg);
void uart_init ();
void uart_putc ( unsigned int c );
unsigned char uart_getc();
//int strlen(const char* str);
void uart_write(const unsigned char* buffer, int size);
void uart_puts(const char* str);
void IntToString(unsigned int _int, char *_char);
void HexToString(unsigned int _int, char *_char);

#endif
