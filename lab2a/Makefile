TEST_ADD_THREADS = 1 2 4 8 12
TEST_ADD_ITERATIONS = 10 20 40 80 100 1000 10000 100000
TEST_ADD_SYNC_OPTIONS = "" m s c
TEST_ADD_YIELD_OPTIONS = "" --yield
CFLAGS = -Wall -Wextra

build: lab2_add #lab2_list

lab2_add: lab2_add.c
	gcc $(CFLAGS) -lpthread -o $@ $^

lab2_list: lab2_list.c
	gcc $(CFLAGS) -o $@ $^

.PHONY: clean dist test graphs

test: lab2_add
	> lab2_add.csv
	for sync_option in $(TEST_ADD_SYNC_OPTIONS) ; do \
		for yield_option in $(TEST_ADD_YIELD_OPTIONS) ; do \
			for threads in $(TEST_ADD_THREADS) ; do \
				for iterations in $(TEST_ADD_ITERATIONS) ; do \
					./lab2_add --threads=$$threads --iterations=$$iterations $$yield_option --sync=$$sync_option >> lab2_add.csv; \
				done \
			done \
		done \
	done

#> lab2_list.csv
graphs: lab2_add.csv #lab2_list.csv
	gnuplot -c lab2_add.gp
#gunplot -c lab2_list.gp lab2_list.csv

clean:
	rm -rf lab2_add lab2_list *.csv *.png *.tar.gz

dist: README lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
	tar -xzf lab2a-404751542.tar.gz $^
