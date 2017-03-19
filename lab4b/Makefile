lab4b: lab4b.c
	gcc -lmraa -lm $^ -o $@

.PHONY: clean dist check

check:

clean:
	rm -f lab4b *.tar.gz

dist: lab4b.c Makefile README
	tar -czf lab4b-404751542.tar.gz $^
