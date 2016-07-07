CFLAGS=-ggdb -Wall -Wextra -Wno-sign-compare
LDLIBS=-lpthread

all: server

server: data.o termcolors.o csvreader.o util.o stats.o serial.o list.o pokegen.o

clean:
	rm -f *.o
