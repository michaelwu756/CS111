CFLAGS=-lmraa -lm -lssl -lcrypto -Wall -Wextra

default: lab4c_tcp lab4c_tls

lab4c_tcp: lab4c_tcp.c
lab4c_tls: lab4c_tls.c

lab4c_tcp lab4c_tls:
	gcc $(CFLAGS) $^ -o $@

.PHONY: default clean dist

clean:
	rm -f lab4c_tcp lab4c_tls *.tar.gz

dist: lab4b_tcp.c lab4b_tls.c Makefile README
	tar -czf lab4c-404751542.tar.gz $^
