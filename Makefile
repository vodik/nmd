CC = gcc
CFLAGS = -Wall -Werror -pedantic -std=gnu99 -O0 -g
LDFLAGS = -lpthread -liw

#SRC = wireless.c scanning.c nmd.c
SRC = nmd.c interfaces.c
OBJ = ${SRC:.c=.o}

all: nmd

nmd: ${OBJ}
	@echo CC -o $@
	@${CC} ${LDFLAGS} -o $@ ${OBJ}

%.o: %.c
	@echo CC $@
	@${CC} -o $@ -c $< ${CFLAGS}

clean:
	@echo cleaning...
	@rm nmd ${OBJ}

.PHONY: all clean
