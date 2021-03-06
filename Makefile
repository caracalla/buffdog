P=buffdog
OBJECTS=device.cpp line.cpp util.cpp
CXXFLAGS=-g -Wall -std=c++17
LDLIBS=-lm -lSDL2
CC=clang++

$(P): $(OBJECTS)

run: clean $(P)
	./$(P)

clean:
	rm -f buffdog spinner delaunay
	rm -rf *.dSYM

spinner: $(OBJECTS)

spin: clean
	make spinner && ./spinner

delaunay: $(OBJECTS) rockshot/triangle.cpp

dt: clean
	make delaunay && ./delaunay
