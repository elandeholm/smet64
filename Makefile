CC=gcc
CC-FLAGS=-Wall -g -std=c99
CC-OPTFLAGS=-O2 -march=native
RM=rm -f

clean:
	$(RM) *~ smet64.o smet64

smet64.o:	smet64.c smet64.h
	$(CC) $(CC-FLAGS) -o smet64.o -c smet64.c

smet64:	smet64.o
	$(CC) $(CC-FLAGS) -o smet64 smet64.o

