TEST_ADD_THREADS = 1 2 4 8 12
TEST_ADD_ITERATIONS = 10 20 40 80 100 1000 10000 100000
TEST_ADD_SYNC_OPTIONS = "" m s c
TEST_ADD_YIELD_OPTIONS = "" --yield

TEST_LIST_1_THREADS = 1
TEST_LIST_1_ITERATIONS = 10 100 1000 10000 20000

TEST_LIST_2_THREADS = 2 4 8 12
TEST_LIST_2_ITERATIONS = 1 10 100 1000

TEST_LIST_3_THREADS = 2 4 8 12
TEST_LIST_3_ITERATIONS = 1 2 4 8 16 32
TEST_LIST_3_YIELD_OPTIONS = "" i d il dl

TEST_LIST_4_THREADS = 12
TEST_LIST_4_ITERATIONS = 32
TEST_LIST_4_YIELD_OPTIONS = i d il dl
TEST_LIST_4_SYNC_OPTIONS = m s

TEST_LIST_5_THREADS = 1 2 4 8 12 16 24
TEST_LIST_5_ITERATIONS = 1000
TEST_LIST_5_SYNC_OPTIONS = m s

CFLAGS = -Wall -Wextra -lpthread

build: lab2_add lab2_list

lab2_add: lab2_add.c
	gcc $(CFLAGS) -o $@ $^

lab2_list: lab2_list.c SortedList.c
	gcc $(CFLAGS) -o $@ $^

lab2_add-%.png: lab2_add.csv lab2_add.gp
	gnuplot -c lab2_add.gp

lab2_list-%.png: lab2_list.csv lab2_list.gp
	gnuplot -c lab2_list.gp

lab2_add.csv: lab2_add
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

lab2_list.csv: lab2_list
	> lab2_list.csv
	for threads in $(TEST_LIST_1_THREADS) ; do \
		for iterations in $(TEST_LIST_1_ITERATIONS) ; do \
			./lab2_list --threads=$$threads --iterations=$$iterations >> lab2_list.csv; \
		done \
	done

	-for threads in $(TEST_LIST_2_THREADS) ; do \
		for iterations in $(TEST_LIST_2_ITERATIONS) ; do \
			./lab2_list --threads=$$threads --iterations=$$iterations >> lab2_list.csv; \
		done \
	done

	-for yield_option in $(TEST_LIST_3_YIELD_OPTIONS) ; do \
		for threads in $(TEST_LIST_3_THREADS) ; do \
			for iterations in $(TEST_LIST_3_ITERATIONS) ; do \
				./lab2_list --threads=$$threads --iterations=$$iterations --yield=$$yield_option >> lab2_list.csv; \
			done \
		done \
	done

	for sync_option in $(TEST_LIST_4_SYNC_OPTIONS) ; do \
		for yield_option in $(TEST_LIST_4_YIELD_OPTIONS) ; do \
			for threads in $(TEST_LIST_4_THREADS) ; do \
				for iterations in $(TEST_LIST_4_ITERATIONS) ; do \
					./lab2_list --threads=$$threads --iterations=$$iterations --yield=$$yield_option --sync=$$sync_option >> lab2_list.csv; \
				done \
			done \
		done \
	done

	for sync_option in $(TEST_LIST_5_SYNC_OPTIONS) ; do \
		for threads in $(TEST_LIST_5_THREADS) ; do \
			for iterations in $(TEST_LIST_5_ITERATIONS) ; do \
				./lab2_list --threads=$$threads --iterations=$$iterations --sync=$$sync_option >> lab2_list.csv; \
			done \
		done \
	done

.PHONY: build clean dist test graphs

tests:
	rm -f lab2_add.csv lab2_list.csv
	make lab2_add.csv lab2_list.csv

graphs:
	rm -f lab2_add-*.png lab2_list-*.png
	make lab2_add-1.png lab2_list-1.png

clean:
	rm -f lab2_add lab2_list *.tar.gz

dist: README lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv lab2_add.gp lab2_list.gp lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png
	tar -czf lab2a-404751542.tar.gz $^
