CC=gcc
ARGS=-Wall -Wextra -std=c99 -MMD
CCFLAGS=$(ARGS) $(GTK)
LDFLAGS=-pthread -lm

SRCR=$(shell find ./Reseau -name '*.c' -not -name "main.c")
SRCG=$(shell find ./Noeud_de_gestion -name '*.c' -not -name "main.c")
SRCM=$(shell find ./Noeud_de_gestion -name '*.c' -not -name "main.c")
SRCW=$(shell find ./Site_web -name '*.c' -not -name "main.c")

SRC += $(shell find -maxdepth 1 -name '*.c')
OBJ=$(shell find -name '*.o')

EXE=$(shell find -mindepth 2 -name '*.out')
EXE +=$(shell find -mindepth 2 -name '*.exe')

NAME=blockchain

all:
	$(MAKE) clean
	$(MAKE) main

main: reseau.o gestion.o minage.o
	$(CC) $(CCFLAGS) -o $(NAME) $(ADD) $^ $(SRC) $(LDFLAGS)


reseau.o:
	$(CC) $(CCFLAGS) -c -o $@ $(SRCR) $(LDFLAGS)

gestion.o:
	$(CC) $(CCFLAGS) -c -o $@ $(SRCG) $(LDFLAGS)

minage.o:
	$(CC) $(CCFLAGS) -c -o $@ $(SRCM) $(LDFLAGS)

web.o:
	$(CC) $(CCFLAGS) -c -o $@ $(SRCW) $(LDFLAGS)
	
clean:
	for i in $(OBJ);do(rm $$i);done
	for i in $(EXE);do(rm $$i);done

cleanall: clean
	rm $(NAME)

.SILENT:all main clean cleanall reseau.o gestion.o minage.o web.o
