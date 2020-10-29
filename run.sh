#!/bin/bash

PARALLEL_FILE=./par_sample_sort
SEQUENTIAL_FILE=./seq_sample_sort
small=16777216
medium=33554432
big=67108864

run(){
	for input in $small $medium $big 
		do
			for processes in 2 4 8 16
				do
					echo "Running sequential algorithm with $processes partations... Input file = $input"
					START=$(date +%s.%N)
					for ((i=1;i<=times;i++)); do
						$SEQUENTIAL_FILE -s $input -n $processes
					done
					END=$(date +%s.%N)
					TIME_SEQ=$(python3 -c "print('{:.2f}'.format(${END} - ${START}))")
					echo "It took $TIME_SEQ seconds to run as input of size $input sequentially with $processes partations"
			
					echo "Running paralell algorithm with $processes processes... Input file = $input"
					START=$(date +%s.%N)
					for ((i=1;i<=times;i++)); do
						mpirun --host arthur:$processes $PARALLEL_FILE -s $input
					done
					END=$(date +%s.%N)
					TIME_PAR=$(python3 -c "print('{:.2f}'.format(${END} - ${START}))")
					echo "It took $TIME_PAR seconds to run $input paralelly with $processes processes"
					SPEEDUP=$(python3 -c "print('{:.2f}'.format(${TIME_SEQ} / ${TIME_PAR}))")
					echo "Speed up for input $input and $processes processes is $SPEEDUP"
					echo ""
				done
		done
}

helpFunction()
{
   echo ""
   echo "Usage: $0 -n "
   echo -e "\t-n number of times that the programas will run "
}


while getopts "n:" opt
do
   case "$opt" in
      n ) times="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

if [ -z "$times" ]
then
   echo "Parameters are empty";
   helpFunction
fi


echo ""

if [ -f "$PARALLEL_FILE" ] && [ -f "$SEQUENTIAL_FILE" ] ; then
	run
else 
    echo "$PARALLEL_FILE or $SEQUENTIAL_FILE do not exist. Please run make before running this script!"
fi
