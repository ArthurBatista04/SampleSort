CXX=mpicc



all: main

main: 
	$(CXX) -O3 ./parallel/samplesort.c -o par_sample_sort
	$(CXX) -O3 ./sequential/sequential.c -o seq_sample_sort
clean:
	rm -f *.o par_sample_sort seq_sample_sort
