P=rockshot
OBJECTS=../device.cpp ../line.cpp ../util.cpp *.cpp
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
