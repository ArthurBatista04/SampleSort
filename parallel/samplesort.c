#include "samplesort.h"

int cmpfunc(const void *a, const void *b)
{
	return (*(double *)a - *(double *)b);
}

int main(int argc, char *argv[])
{
	int num_tasks,
		task_id,
		num_workers,
		sample_vec_size,
		size;
	int opt, rc;
	double **buckets;
	double *vector, *sample_vec, *local_splitter, *global_splitter;
	int *bucket_sizes;
	int local_splitter_element_size, global_splitter_size;
	time_t t;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

	if (num_tasks < 2)
	{
		printf("Need at least two MPI tasks. Quitting...\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
		exit(1);
	}

	if (task_id == MASTER)
	{

		while ((opt = getopt(argc, argv, "s:h")) != -1)
		{
			switch (opt)
			{
			case 's':
				size = atoi(optarg);
				if ((size % num_tasks) != 0)
				{

					printf("Number of elements are not divisible by number of processes \n");
					MPI_Finalize();
					exit(0);
				}
				break;
			case 'h':
				printf("Usage: mpirun %s -np (nº processes) -s (size input) -h (help) \n", argv[0]);
				MPI_Finalize();
				exit(EXIT_SUCCESS);
			case '?':
				MPI_Finalize();
				exit(EXIT_FAILURE);
			default:
				MPI_Finalize();
				abort();
			}
		}
		vector = (double *)calloc(size, sizeof(double));
		srand(time(NULL));
		printf("\n---------------INICIAL VECTOR-----------------\n");
		for (int i = 0; i < size; i++)
		{
			vector[i] = (double)rand() / (double)RAND_MAX;
			printf("%f ", vector[i]);
		}
		printf("\n\n");
	}

	MPI_Bcast(&size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	sample_vec_size = size / num_tasks;
	sample_vec = (double *)calloc(sample_vec_size, sizeof(double));

	MPI_Scatter(vector, sample_vec_size, MPI_DOUBLE, sample_vec,
				sample_vec_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

	// printf("PROCESS %d SUB-VECTOR: ", task_id);
	// for (int i = 0; i < sample_vec_size; i++)
	// {
	// 	printf("%f ", sample_vec[i]);
	// }
	// printf("\n\n");

	qsort(sample_vec, sample_vec_size, sizeof(double), cmpfunc);

	printf("PROCESS %d SUB-VECTOR: ", task_id);
	for (int i = 0; i < sample_vec_size; i++)
	{
		printf("%f ", sample_vec[i]);
	}
	printf("\n");

	local_splitter_element_size = num_tasks - 1;
	local_splitter = (double *)calloc(local_splitter_element_size, sizeof(double));
	for (int i = 0; i < local_splitter_element_size; i++)
	{
		local_splitter[i] = sample_vec[size / num_tasks / num_tasks * (i + 1)];
	}

	// printf("PROCESS %d SUB-VECTOR: ", task_id);
	// for (int i = 0; i < local_splitter_element_size; i++)
	// {
	// 	printf("%f ", local_splitter[i]);
	// }
	// printf("\n");
	global_splitter_size = local_splitter_element_size * num_tasks;
	global_splitter = (double *)calloc(global_splitter_size, sizeof(double));
	MPI_Gather(local_splitter, local_splitter_element_size, MPI_DOUBLE, global_splitter, local_splitter_element_size,
			   MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	if (task_id == MASTER)
	{

		qsort(global_splitter, global_splitter_size, sizeof(double), cmpfunc);
		printf("Global Splitter: ");
		for (int i = 0; i < global_splitter_size; i++)
		{
			printf("%f ", global_splitter[i]);
		}
	}
	MPI_Bcast(global_splitter, global_splitter_size, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

	buckets = (double **)malloc(sizeof(double *) * global_splitter_size);
	for (int i = 0; i < global_splitter_size; i++)
	{
		buckets[i] = (double *)malloc(sizeof(double) * 2 * size / global_splitter_size);
	}

	bucket_sizes = (int *)calloc(global_splitter_size, sizeof(int));

	for (int i = 0; i < sample_vec_size; i++)
	{
		int count = 0;
		while (count < global_splitter_size)
		{
			if (sample_vec[i] < global_splitter[count])
			{
				buckets[count][bucket_sizes[count]] = sample_vec[i];
				bucket_sizes[count]++;
				count = global_splitter_size; // break
			}
			count++;
		}
	}
	if (task_id == MASTER)
	{

		for (int i = 0; i < global_splitter_size; i++)
		{
			printf("\n\n %d BUCKET NUMERO %d\n\n", task_id, i);
			for (int j = 0; j < bucket_sizes[i]; j++)
			{
				printf("%f ", buckets[i][j]);
			}
			printf("\n\n");
		}
	}

	MPI_Finalize();
	return 0;
}
