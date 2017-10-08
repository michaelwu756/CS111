all: lab1a

lab1a: lab1a.c
	gcc -Wall -Wextra lab1a.c -o lab1a

.PHONY: clean dist

clean:
	rm -f lab1a lab1a-404751542.tar.gz

dist: Makefile lab1a.c README
	tar -czf lab1a-404751542.tar.gz $^
