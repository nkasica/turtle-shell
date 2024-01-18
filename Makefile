CC = gcc
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow -pedantic-errors -fstack-protector-all -Wextra

all: turtle

turtle: turtle.o
        $(CC) -o turtle turtle.o

turtle.o: turtle.c
        $(CC) $(CFLAGS) -c turtle.c

clean:
        @rm -f *.o turtle
