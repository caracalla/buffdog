P=rockshot
OBJECTS=../device.cpp ../line.cpp ../util.cpp model.cpp player.cpp scene.cpp triangle.cpp
CXXFLAGS=-g -Wall -std=c++17
LDLIBS=-lm -lSDL2
CC=clang++

$(P): $(OBJECTS)

run: clean $(P)
	./$(P)

debug:
	make
	lldb $(P)

clean:
	rm -f $(P)
	rm -rf *.dSYM

.PHONY: wad bsp

wad:
	rm -f wad && $(CC) $(CXXFLAGS) -o wad ../util.cpp wad.cpp && ./wad

bsp:
	rm -f bsp && $(CC) $(CXXFLAGS) -o bsp ../util.cpp bsp.cpp && ./bsp
