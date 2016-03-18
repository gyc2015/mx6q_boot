
include $(TOPDIR)/config.mk

BOOT_START = $(obj)start.o
BOOT_DCD = $(obj)dcd.bin

all: $(BOOT_DCD) $(BOOT_START) $(COBJS) $(SOBJS)

$(BOOT_DCD): dcd.o
	$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend


