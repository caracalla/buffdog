P=buffdog
OBJECTS=device.cpp line.cpp
CXXFLAGS=-g -Wall -std=c++17
LDLIBS=-lm -lSDL2
CC=clang++

$(P): $(OBJECTS)

run: clean $(P)
	./$(P)

clean:
	rm -f buffdog
	rm -rf *.dSYM
