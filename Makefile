P=buffdog
OBJECTS=device.c line.c
CFLAGS=-g -Wall -O3
LDLIBS=-lm
CC=c99

$(P): $(OBJECTS)