CFLAGS=-lmraa -lm -Wall -Wextra

default: lab4b

lab4b: lab4b.c
	gcc $(CFLAGS) $^ -o $@

.PHONY: default clean dist check

check: lab4b
	echo "BOGUS\nOFF" | ./lab4b
	(sleep 2; echo "PERIOD=2"; sleep 3; echo "OFF") | ./lab4b
	(sleep 1; echo "SCALE=C"; sleep 1; echo "SCALE=F"; sleep 1; echo "OFF") | ./lab4b --scale=F
	(sleep 1; echo "STOP"; sleep 2; echo "START"; sleep 1; echo "OFF") | ./lab4b
	(sleep 3; echo "OFF") | ./lab4b --scale=C --period=2
	(sleep 1; echo "OFF") | ./lab4b --log=LOGFILE
	cat LOGFILE
	rm LOGFILE

clean:
	rm -f lab4b *.tar.gz LOGFILE

dist: lab4b.c Makefile README
	tar -czf lab4b-404751542.tar.gz $^
