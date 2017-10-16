CFLAGS=-Wall -Wextra -lmcrypt

all: lab1b-client lab1b-server

lab1b-client: lab1b-client.c
	gcc $(CFLAGS) -o $@ $<

lab1b-server: lab1b-server.c
	gcc $(CFLAGS) -o $@ $<

.PHONY: clean dist

clean:
	rm -f lab1b-server lab1b-client lab1b-404751542.tar.gz

dist: my.key README lab1b-client.c lab1b-server.c Makefile
	tar -czf lab1b-404751542.tar.gz $^
