include ../Makefile.inc

CPPFLAGS= -I../include

PROG=		kernel

all:
	make $(PROG).img
	mountU.sh

LDFLAGS = -mcpu=arm1176jzf-s -fpic -ffreestanding
OBJS =	entry.o \
	timer.o machdep.o task.o \
	page.o startup.o frame.o kmalloc.o \
	printk.o bitmap.o uart.o interrupt.o
OBJS += ../lib/string.o ../lib/memcpy.o \
		../lib/memset.o  ../lib/tlsf/tlsf.o

$(PROG).img: $(PROG).elf
	$(OBJCOPY) $^ -O binary $@

$(PROG).elf: $(OBJS)
	$(CC) -T ldscript.in -o$@ -ffreestanding -O2 -nostdlib $(OBJS)

$(OBJS):../include/*.h ../include/sys/*.h

.PHONY: clean
clean:
	-$(RM) ldscript $(OBJS) $(PROG).img $(PROG).elf $(PROG).map *.*~
