CC = gcc
CPPFLAGS =

CFLAGS = -O2 -g -Wall -Wextra -Wformat=2 
CFLAGS = `pkg-config --cflags glib-2.0`
LDLIBS = `pkg-config --libs glib-2.0`

all: httpd

clean:
	rm -f *.o *~

distclean: clean
	rm -f httpd
