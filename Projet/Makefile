CC=gcc
ARGS=-Wall -Wextra -std=c99
GTK= -pthread -pipe
CCFLAGS=$(ARGS) $(GTK)
LDFLAGS=-pthread -lm

SRCR=$(shell find ./Reseau -mindepth 2 -name '*.c' -not -name "main.c")
SRCG=$(shell find ./Noeud_de_gestion -mindepth 2 -name '*.c' -not -name "main.c")
SRCM=$(shell find ./Noeud_de_gestion -mindepth 2 -name '*.c' -not -name "main.c")
SRCW=$(shell find ./Site_web -mindepth 2 -name '*.c' -not -name "main.c")

SRC += $(shell find -maxdepth 1 -name '*.c')
OBJ=$(shell find -name '*.o')

EXE=$(shell find -mindepth 2 -name '*.out')
EXE +=$(shell find -mindepth 2 -name '*.exe')

all:
	$(MAKE) main

main: reseau gestion minage web
	$(CC) $(CCFLAGS) -o blockchain $(ADD) $(OBJ) $(SRC) $(LDFLAGS)


reseau:
	$(CC) $(CCFLAGS) -c -o reseau.o $(SRCR) $(LDFLAGS)

gestion:
	$(CC) $(CCFLAGS) -c -o gestion.o $(SRCG) $(LDFLAGS)

minage:
	$(CC) $(CCFLAGS) -c -o minage.o $(SRCM) $(LDFLAGS)

web:
	$(CC) $(CCFLAGS) -c -o web.o $(SRCW) $(LDFLAGS)
	
clean:
	for i in $(OBJ);do(rm $$i);done
	for i in $(EXE);do(rm $$i);done

.SILENT:all main clean copy
