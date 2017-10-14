CFLAGS=-Wall -Wextra

all: lab1b-client lab1b-server

lab1b-client: lab1b-client.c

	gcc $(CFLAGS) -o $@ $<

lab1b-server: lab1b-server.c

	gcc $(CFLAGS) -o $@ $<

.PHONY: clean dist

clean:

	rm -f lab1b-server lab1b-client

dist:
