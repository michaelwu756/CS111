#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#	8. average lock acquisition time (ns)
#
# output:
#	lab2b_1.png ... throughput vs. number of threads for mutex and spin-lock synchronized list operations.
#	lab2b_2.png ... mean time per mutex wait and mean time per operation for mutex-synchronized list operations.
#	lab2b_3.png ... successful iterations vs. threads for each synchronization method.
#	lab2b_4.png ... throughput vs. number of threads for mutex synchronized partitioned lists.
#	lab2b_5.png ... throughput vs. number of threads for spin-lock-synchronized partitioned lists.
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# Throughput vs. Threads
set title "List-1: Throughput vs. Number of Threads"
set xlabel "Threads"
set xrange [0.75:]
set logscale x 2
set ylabel "Throughput (Operations/Second)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only unprotected mutex and spin locked runs with 1000 iterations and 1 list
plot \
     "< head -14 lab2b_list.csv | grep -e 'list-none-m,[0-9]*,1000,1,'" using ($2):(1000000000/($7)) \
	title 'mutex, 1000 iterations' with linespoints lc rgb 'red', \
     "< head -14 lab2b_list.csv | grep -e 'list-none-s,[0-9]*,1000,1,'" using ($2):(1000000000/($7)) \
	title 'spin, 1000 iterations' with linespoints lc rgb 'green'

set title "List-2: Time Waiting for Mutex and Time per Operation"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Time (ns)"
set logscale y 10
set output 'lab2b_2.png'
plot \
     "< head -14 lab2b_list.csv | grep -e 'list-none-m,[0-9]*,1000,1,'" using ($2):($8) \
	title 'Average wait for mutex time, 1000 iterations' with linespoints lc rgb 'red', \
     "< head -14 lab2b_list.csv | grep -e 'list-none-m,[0-9]*,1000,1,'" using ($2):($7) \
	title 'Average time per operation, 1000 iterations' with linespoints lc rgb 'green'

set title "List-3: Iterations sucessfully run, Lists=4, Yield=id"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:17]
set ylabel "Successful Iterations"
set logscale y 10
set yrange [0.75:]
set output 'lab2b_3.png'
plot \
     "< grep -e 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	with points pointtype 1 lc rgb "red" title "Unprotected", \
     "< grep -e 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	with points pointtype 2 lc rgb "green" title "Mutex", \
     "< grep -e 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	with points pointtype 6 lc rgb "blue" title "Spin-Lock"
#
# "no valid points" is possible if even a single iteration can't run
#

unset yrange
set title "List-4: Throughput vs. Number of Threads, Mutex with Sublists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (Operations/Second)"
set logscale y 10
set output 'lab2b_4.png'
plot \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-m,[0-9]*,1000,1,'" using ($2):(1000000000/($7)) \
	title 'Lists=1' with linespoints lc rgb 'red', \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Lists=4' with linespoints lc rgb 'green', \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Lists=8' with linespoints lc rgb 'blue', \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Lists=16' with linespoints lc rgb 'purple'

set title "List-5: Throughput vs. Number of Threads, Spin-Lock with Sublists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (Operations/Second)"
set logscale y 10
set output 'lab2b_5.png'
plot \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-s,[0-9]*,1000,1,'" using ($2):(1000000000/($7)) \
	title 'Lists=1' with linespoints lc rgb 'red', \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-s,[0-9]*,1000,4,'" using ($2):(1000000000/($7)) \
	title 'Lists=4' with linespoints lc rgb 'green', \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-s,[0-9]*,1000,8,'" using ($2):(1000000000/($7)) \
	title 'Lists=8' with linespoints lc rgb 'blue', \
     "< tail -40 lab2b_list.csv | grep -e 'list-none-s,[0-9]*,1000,16,'" using ($2):(1000000000/($7)) \
	title 'Lists=16' with linespoints lc rgb 'purple'