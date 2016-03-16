
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o,$(notdir ${SRC}))
ITR = $(patsubst %.c, %.i,$(notdir ${SRC}))
LAB = 

CC = gcc

DBG_FLAG = -DDEBUG
CFLAGS = -std=gnu99 -D_GNU_SOURCE -Wno-trigraphs -pipe -ggdb -I${DIR_INC} ${DBG_FLAG}

%.o : %.c
	$(CC) -c $*.c -o $@ $(CFLAGS) $(CONFCPPFLAGS)

%.i : %.c
	$(CC) -E $*.c -o $@ $(CFLAGS) $(CONFCPPFLAGS)
