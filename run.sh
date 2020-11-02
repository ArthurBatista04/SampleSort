#!/bin/bash

PARALLEL_FILE=./par_sample_sort
SEQUENTIAL_FILE=./seq_sample_sort
small=33554432
medium=67108864
big=134217728

sequential_times=()
sequential_time=0
parallel_times=()
parallel_time=0

run() {
	for input in $small $medium $big ; do

		echo "Running sequential algorithm with 4 partations... Input size = $input"
		for ((i = 1; i <= times; i++)); do
			START=$(date +%s.%N)
			$SEQUENTIAL_FILE -s $input -n 4
			END=$(date +%s.%N)
			TIME_SEQ=$(python3 -c "print('{:.2f}'.format(${END} - ${START}))")
			sequential_times[$i]=$TIME_SEQ
		done
		
		for ((i = 1; i <= times; i++)); do
			sequential_time=$(python3 -c "print('{:.2f}'.format(${sequential_time} + ${sequential_times[$i]}))")
		done
		sequential_time=$(python3 -c "print('{:.2f}'.format(${sequential_time} / ${times}))")
		
		echo "It took in average of $sequential_time seconds to run $input paralelly with 4 processes"
		echo "Running paralell algorithm with 4 processes... Input size = $input"
		for ((i = 1; i <= times; i++)); do
			START=$(date +%s.%N)
			mpirun -np 4 $PARALLEL_FILE -s $input
			END=$(date +%s.%N)
			TIME_PAR=$(python3 -c "print('{:.2f}'.format(${END} - ${START}))")
			parallel_times[$i]=$TIME_PAR
		done
		for ((i = 1; i <= times; i++)); do
			parallel_time=$(python3 -c "print('{:.2f}'.format(${parallel_time} + ${parallel_times[$i]}))")
		done

		parallel_time=$(python3 -c "print('{:.2f}'.format(${parallel_time} / ${times}))")

		echo "It took in average of $parallel_time seconds to run $input paralelly with 4 processes"

		SPEEDUP=$(python3 -c "print('{:.2f}'.format(${sequential_time} / ${parallel_time}))")
		echo "Speed up for input $input and 4 processes is $SPEEDUP"
		echo ""
		sequential_time=0
		parallel_time=0

	done
}

helpFunction() {
	echo ""
	echo "Usage: $0 -n "
	echo -e "\t-n number of times that the programas will run "
}

while getopts "n:" opt; do
	case "$opt" in
	n) times="$OPTARG" ;;
	?) helpFunction ;; # Print helpFunction in case parameter is non-existent
	esac
done

if [ -z "$times" ]; then
	echo "Parameters are empty"
	helpFunction
fi

echo ""

if [ -f "$PARALLEL_FILE" ] && [ -f "$SEQUENTIAL_FILE" ]; then
	run
else
	echo "$PARALLEL_FILE or $SEQUENTIAL_FILE do not exist. Please run make before running this script!"
fi
