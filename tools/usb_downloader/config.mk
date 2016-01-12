
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o,$(notdir ${SRC}))
LAB = -lusb-1.0

CC = gcc

DBG_FLAG = -DDEBUG
CFLAGS = -std=gnu99 -D_GNU_SOURCE -Wno-trigraphs -pipe -ggdb -I${DIR_INC} -I/usr/include/libusb-1.0 ${DBG_FLAG}

%.o : %.c
	$(CC) -c $*.c -o $@ $(CFLAGS) $(CONFCPPFLAGS)

%.i : %.c
	$(CC) -E $*.c -o $@ $(CFLAGS) $(CONFCPPFLAGS)
