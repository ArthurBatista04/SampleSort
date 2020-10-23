CXX=mpicc



all: main

main: 
	$(CXX) ./parallel/samplesort.c -o par_sample_sort
clean:
	rm -f *.o par_sample_sort
