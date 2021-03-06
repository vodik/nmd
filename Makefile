CC = clang
CFLAGS = -Wall -Werror -pedantic -std=gnu99 -O0 -g
LDFLAGS = -lpthread -liw

SRC = nmd.c interfaces.c scanning.c connect.c commands.c server.c epoll.c
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
