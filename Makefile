all: disassemble

CC=gcc
CLIBS=-lc
CFLAGS=-g -Wall -pedantic -std=c99

DISASSEMBLEOBJS=disassembler.o printRoutines.o

disassemble: $(DISASSEMBLEOBJS)
	$(CC) -g -o disassemble $(DISASSEMBLEOBJS)

disassembler.o: disassembler.c printRoutines.h
printRoutines.o: printRoutines.c printRoutines.h

clean:
	-rm -rf *.o disassemble
