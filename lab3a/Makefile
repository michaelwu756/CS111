CFLAGS = -Wall -Wextra

default: lab3a

lab3a: lab3a.c
	gcc $(CFLAGS) $^ -o $@

.PHONY: default clean dist

clean:
	rm -f lab3a *.tar.gz

dist: lab3a.c Makefile README ext2_fs.h
	tar -czf lab3a-404751542.tar.gz $^
