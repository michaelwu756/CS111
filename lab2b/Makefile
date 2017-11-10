TEST_LIST_1_ITERATIONS = 1000
TEST_LIST_1_THREADS = 1 2 4 6 8 16 24
TEST_LIST_1_SYNC_OPTIONS = m s

TEST_LIST_2_ITERATIONS = 1 2 4 8 16
TEST_LIST_2_THREADS = 1 4 8 12 16
TEST_LIST_2_YIELD_OPTIONS = id
TEST_LIST_2_LISTS = 4

TEST_LIST_3_ITERATIONS = 10 20 40 80
TEST_LIST_3_THREADS = 1 4 8 12 16
TEST_LIST_3_YIELD_OPTIONS = id
TEST_LIST_3_SYNC_OPTIONS = m s
TEST_LIST_3_LISTS = 4

TEST_LIST_4_ITERATIONS = 1000
TEST_LIST_4_THREADS = 1 2 4 8 12
TEST_LIST_4_SYNC_OPTIONS = m s
TEST_LIST_4_LISTS = 1 4 8 16

CFLAGS = -Wall -Wextra -lpthread -g

default: lab2_list

lab2_list: lab2_list.c SortedList.c
	gcc $(CFLAGS) -o $@ $^

lab2b_list.csv: lab2_list
	> $@
	for sync_option in $(TEST_LIST_1_SYNC_OPTIONS) ; do \
		for threads in $(TEST_LIST_1_THREADS) ; do \
			for iterations in $(TEST_LIST_1_ITERATIONS) ; do \
				./lab2_list --threads=$$threads --iterations=$$iterations --sync=$$sync_option >> $@; \
			done \
		done \
	done

	-for list in $(TEST_LIST_2_LISTS) ; do \
		for yield_option in $(TEST_LIST_2_YIELD_OPTIONS) ; do \
			for threads in $(TEST_LIST_2_THREADS) ; do \
				for iterations in $(TEST_LIST_2_ITERATIONS) ; do \
					./lab2_list --threads=$$threads --iterations=$$iterations --yield=$$yield_option --lists=$$list >> $@; \
				done \
			done \
		done \
	done

	for list in $(TEST_LIST_3_LISTS) ; do \
		for sync_option in $(TEST_LIST_3_SYNC_OPTIONS) ; do \
			for yield_option in $(TEST_LIST_3_YIELD_OPTIONS) ; do \
				for threads in $(TEST_LIST_3_THREADS) ; do \
					for iterations in $(TEST_LIST_3_ITERATIONS) ; do \
						./lab2_list --threads=$$threads --iterations=$$iterations --yield=$$yield_option --sync=$$sync_option --lists=$$list >> $@; \
					done \
				done \
			done \
		done \
	done

	for list in $(TEST_LIST_4_LISTS) ; do \
		for sync_option in $(TEST_LIST_4_SYNC_OPTIONS) ; do \
			for threads in $(TEST_LIST_4_THREADS) ; do \
				for iterations in $(TEST_LIST_4_ITERATIONS) ; do \
					./lab2_list --threads=$$threads --iterations=$$iterations --sync=$$sync_option --lists=$$list >> $@; \
				done \
			done \
		done \
	done

lab2b_%.png: lab2b_list.csv lab2b_list.gp
	gnuplot -c lab2b_list.gp

profile.out: lab2_list Makefile
	-rm -f ./raw.gperf
	LD_PRELOAD=/u/cs/ugrad/wum/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > $@
	pprof --list=FILLIN ./lab2_list ./raw.gperf >> $@
	rm -f ./raw.gperf

.PHONY: default clean dist test graphs profile

tests:
	rm -f lab2b_list.csv
	make lab2b_list.csv

graphs:
	rm -f lab2b_*.png
	make lab2b_1.png

profile:
	rm -f profile.out
	make profile.out

clean:
	rm -f lab2_list *.csv *.png *.tar.gz profile.out

dist: README SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv lab2b_list.gp lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png
	tar -czf lab2b-404751542.tar.gz $^
