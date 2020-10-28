CXX=mpicc



all: main

main: 
	$(CXX) ./parallel/samplesort.c -o par_sample_sort
	$(CXX) ./sequential/sequential.c -o seq_sample_sort
clean:
	rm -f *.o par_sample_sort seq_sample_sort
