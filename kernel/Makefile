include ../Makefile.inc

CPPFLAGS= -I../include
CFLAGS=-fno-builtin

PROG=		kernel

all:
	make $(PROG).img
	mountU.sh

LDFLAGS = -Wall -O2 -nostdlib -nostartfiles -ffreestanding
OBJS =	entry.o \
	timer.o machdep.o task.o \
	page.o startup.o frame.o kmalloc.o \
	printk.o bitmap.o uart.o interrupt.o
OBJS += ../lib/string.o ../lib/memcpy.o \
		../lib/memset.o  ../lib/tlsf/tlsf.o \
		../lib/snprintf.o

$(PROG).img: $(PROG).elf
	$(OBJDUMP) -D $(PROG).elf > $(PROG).list
	$(OBJCOPY) $^ -O binary $@

$(PROG).elf: $(OBJS) ldscript.in
	$(LD) $(OBJS) -L../lib/ -lgcc -T ldscript.in -o $@

$(OBJS):../include/*.h ../include/sys/*.h

.PHONY: clean
clean:
	-$(RM) ldscript $(OBJS) $(PROG).img $(PROG).elf $(PROG).map *.*~
