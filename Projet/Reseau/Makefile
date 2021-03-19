# Makefile

CPPFLAGS = -MMD
CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS =
LDLIBS =

SRC = reseau.c
OBJ = ${SRC:.c=.o}
DEP = ${SRC:.c=.d}

all:
	${CC} ${CFLAGS} reseau.c ${CPPFLAGS}

-include ${DEP}

.PHONY: clean

clean:
	${RM} ${OBJ}
	${RM} ${DEP}
	${RM} echo_test single_server loop_server fork_server

# END
