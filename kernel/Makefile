DEFAULT_HOST!=../default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

CFLAGS?=-02 -g
LDFLAGS?=
LIBS?=

PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=$(KERNEL_ARCH_OBJS) kernel/kernel.o

OBJS=$(ARCHDIR)/crti.o $(ARCHDIR)/crtbegin.o $(KERNEL_OBJS)\
$(ARCHDIR)/crtend.o $(ARCHDIR)/crtn.o \

LINK_LIST=\
$(LDFLAGS) $(ARCHDIR)/crti.o $(ARCHDIR)/crtbegin.o $(KERNEL_OBJS) \
$(LIBS) $(ARCHDIR)/crtend.o $(ARCHDIR)/crtn.o \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: simple.kernel

simple.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -Wl,-Map=output.map -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot simple.kernel

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS)

clean:
	rm -f simple.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d */*/*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)/kernel/.
	cp -R --preserve=timestamps -t $(DESTDIR)$(INCLUDEDIR)/kernel/. $(filter-out include/kernel/arch/%, $(shell find include/ -name "*.h"))
	cp -R --preserve=timestamps include/kernel/arch/$(HOSTARCH)/. $(DESTDIR)$(INCLUDEDIR)/kernel/.

install-kernel: simple.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp simple.kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
