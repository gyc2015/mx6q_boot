
DIR_ROOT := $(CURDIR)
DIR_INC := $(CURDIR)/inc
export DIR_ROOT DIR_INC

include ${DIR_ROOT}/config.mk

elf_loader: ${OBJ} ${ITR}
	$(CC) $(OBJ) $(LAB) -o $@

clean:
	@find $(DIR_ROOT) -type f \
		\( -name 'elf_loader' -o -name '*.bak' -o -name '*~' \
		-o -name '*.o'	-o -name '*.a' -o -name '*.exe' -o -name '*.s'	\
		-o -name '*.i' \) -print \
		| xargs rm -f
