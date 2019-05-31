P=buffdog
OBJECTS=device.c line.c vec3.c sphere.c light.c util.c
CFLAGS=-g -Wall -O3
LDLIBS=-lm
CC=c99

$(P): $(OBJECTS)

run: $(P)
	./$(P)

spinner: $(OBJECTS)
