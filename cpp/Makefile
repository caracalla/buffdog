P=buffdog
OBJECTS=device.cpp line.cpp util.cpp
CFLAGS=-g -Wall -O3
LDLIBS=-lm -lSDL2
CC=clang++

$(P): $(OBJECTS)

run: clean $(P)
	./$(P)

clean:
	rm -f buffdog
	rm -rf *.dSYM
