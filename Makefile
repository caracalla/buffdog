P=buffdog
OBJECTS=device.c line.c vec3.c sphere.c light.c util.c
CFLAGS=-g -Wall -O3
LDLIBS=-lm -lSDL2
CC=clang

$(P): $(OBJECTS)

run: $(P)
	./$(P)

spinner: $(OBJECTS)

ray_tracer: $(OBJECTS)

clean:
	rm -f spinner
	rm -f ray_tracer
	rm -rf *.dSYM
