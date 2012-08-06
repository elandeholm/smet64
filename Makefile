CC=gcc
CC-FLAGS=-Wall -g -std=c99
CC-OPTFLAGS=-O2 -march=native
RM=rm -f

clean:
	$(RM) *~ *.o smet64_test

smet64.o:	smet64.c smet64.h
	$(CC) $(CC-FLAGS) $(CC-OPTFLAGS) -o smet64.o -c smet64.c

smet64_test.o: smet64.h smet64_test.c
	$(CC) $(CC-FLAGS) -o smet64_test.o -c smet64_test.c

smet64_test:	smet64_test.o smet64.o
	$(CC) $(CC-FLAGS) -o smet64_test smet64_test.o smet64.o

