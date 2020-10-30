#!/bin/bash

PARALLEL_FILE=./par_sample_sort
SEQUENTIAL_FILE=./seq_sample_sort
small=16777216
medium=33554432
big=67108864

run() {
	for input in $small $medium $big; do

		echo "Running sequential algorithm with 4 partations... Input size = $input"
		START=$(date +%s.%N)
		for ((i = 1; i <= times; i++)); do
			$SEQUENTIAL_FILE -s $input -n 4
		done
		END=$(date +%s.%N)
		TIME_SEQ=$(python3 -c "print('{:.2f}'.format(${END} - ${START}))")
		echo "It took $TIME_SEQ seconds to run as input of size $input sequentially with 4 partations"

		echo "Running paralell algorithm with 4 processes... Input size = $input"
		START=$(date +%s.%N)
		for ((i = 1; i <= times; i++)); do
			mpirun -np 4 $PARALLEL_FILE -s $input
		done
		END=$(date +%s.%N)
		TIME_PAR=$(python3 -c "print('{:.2f}'.format(${END} - ${START}))")
		echo "It took $TIME_PAR seconds to run $input paralelly with 4 processes"
		SPEEDUP=$(python3 -c "print('{:.2f}'.format(${TIME_SEQ} / ${TIME_PAR}))")
		echo "Speed up for input $input and 4 processes is $SPEEDUP"
		echo ""

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
