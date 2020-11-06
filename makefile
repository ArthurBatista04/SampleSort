CXX=mpicc



all: main

main: 
	$(CXX) -O3 ./parallel/samplesort.c -o par_sample_sort
	$(CXX) -O3 ./sequential/sequential.c -o seq_sample_sort
	cd ./relatorio && pdflatex main.tex
clean:
	rm -f *.o par_sample_sort seq_sample_sort relatorio/main.pdf
