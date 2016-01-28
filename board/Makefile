
include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).a

$(LIB): $(COBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(COBJS) $(SOBJS)

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend
