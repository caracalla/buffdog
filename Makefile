P=buffdog
OBJECTS=device.c line.c vec3.c
CFLAGS=-g -Wall -O3
LDLIBS=-lm
CC=c99

$(P): $(OBJECTS)

spinner: $(OBJECTS)

sdl:
	clang sdl.c -o sdl -lSDL2 && ./sdl
