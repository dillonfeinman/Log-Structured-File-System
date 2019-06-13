CFLAGS = -g -Wall -Wextra -DDebug
SHELL:=/bin/bash
P = lfs

all: lfs.o

lfs.o: lfs.cpp inode.h seg.h
	g++ $(CFLAGS) $(P).cpp -o $(P)

run: all
	./$(P)

clean:
	rm -rf *.o $(P)
	rm -rf ./DRIVE
	
checkmem: all
	valgrind ./$(P)