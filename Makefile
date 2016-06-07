CFLAGS=-ggdb -Werror -Wall -Wextra -Wno-sign-compare
LDLIBS=-lpthread

all: server

server: data.o termcolors.o csvreader.o util.o stats.o list.o serial.o pokegen.o

clean:
	rm -f *.o
