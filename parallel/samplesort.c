#include "samplesort.h"

int cmpfunc(const void *a, const void *b)
{
	return (*(int *)a - *(int *)b);
}

int *init_vector(int size)
{
	int count = 0;
	int *vector = (int *)calloc(size, sizeof(int));
	for (int i = size; i >= 0; i--)
	{
		vector[count++] = i;
	}

	return vector;
}

int main(int argc, char *argv[])
{
	int num_tasks,
		task_id,
		num_workers,
		sample_vec_size,
		size;
	int opt, rc;
	int **local_buckets, **global_buckets;
	int *vector, *sample_vec, *local_splitter, *global_splitter, *splitter_buffer, *output;
	int *local_ordered_bucket;
	int *local_bucket_sizes, *global_bucket_buffer_sizes, *displ, *global_bucket_sizes;
	int local_splitter_element_size, global_splitter_size, splitter_buffer_size;
	int print = FALSE;
	int count;
	struct timeval begin, begin_2, begin_3, begin_4, begin_5, end, end_2, end_3, end_4, end_5;
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
		while ((opt = getopt(argc, argv, "s:hp")) != -1)
		{
			switch (opt)
			{
			case 's':
				size = atoi(optarg);
				if ((size % num_tasks) != 0)
				{
					printf("Number of elements are not divisible by number of processes \n");
					MPI_Abort(MPI_COMM_WORLD, rc);
					exit(0);
				}
				break;
			case 'h':
				printf("Usage: mpirun %s -np (nº processes) -s (size input) -h (help) -p (print result) \n", argv[0]);
				MPI_Abort(MPI_COMM_WORLD, rc);
				exit(EXIT_SUCCESS);
				break;
			case 'p':
				print = TRUE;
				break;
			case '?':
				MPI_Abort(MPI_COMM_WORLD, rc);
				exit(EXIT_FAILURE);
				break;
			default:
				MPI_Abort(MPI_COMM_WORLD, rc);
				abort();
				break;
			}
		}
		vector = init_vector(size);
	}

	MPI_Bcast(&size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	// initiliazations
	output = (int *)calloc(size, sizeof(int));
	sample_vec_size = size / num_tasks;
	sample_vec = (int *)calloc(sample_vec_size, sizeof(int));

	local_splitter_element_size = num_tasks - 1;
	local_splitter = (int *)calloc(local_splitter_element_size, sizeof(int));

	global_splitter_size = num_tasks;
	global_splitter = (int *)calloc(global_splitter_size, sizeof(int));

	splitter_buffer_size = local_splitter_element_size * num_tasks;
	splitter_buffer = (int *)calloc(splitter_buffer_size, sizeof(int));

	local_buckets = (int **)malloc(sizeof(int *) * global_splitter_size);
	for (int i = 0; i < global_splitter_size; i++)
	{
		local_buckets[i] = (int *)malloc(sizeof(int) * 2 * size / global_splitter_size);
	}

	local_bucket_sizes = (int *)calloc(global_splitter_size, sizeof(int));

	global_bucket_buffer_sizes = (int *)calloc(global_splitter_size, sizeof(int));
	displ = (int *)calloc(global_splitter_size, sizeof(int));
	global_buckets = (int **)malloc(sizeof(int *) * global_splitter_size);

	for (int i = 0; i < global_splitter_size; i++)
	{
		global_buckets[i] = (int *)malloc(sizeof(int) * 2 * size / global_splitter_size);
	}

	if (task_id == MASTER)
	{
		gettimeofday(&begin, 0);
	}
	// send partitioned vector => sample vector
	MPI_Scatter(vector, sample_vec_size, MPI_INT, sample_vec,
				sample_vec_size, MPI_INT, MASTER, MPI_COMM_WORLD);
	if (task_id == MASTER)
	{
		gettimeofday(&begin_2, 0);
	}
	qsort(sample_vec, sample_vec_size, sizeof(int), cmpfunc);

	// creating local splitters in each process
	for (int i = 0; i < local_splitter_element_size; i++)
	{
		local_splitter[i] = sample_vec[size / (num_tasks * num_tasks) * (i + 1)];
	}
	if (task_id == MASTER)
	{
		gettimeofday(&end_2, 0);
	}
	// gathering each local splitter into a global one
	MPI_Gather(local_splitter, local_splitter_element_size, MPI_INT, splitter_buffer, local_splitter_element_size,
			   MPI_INT, MASTER, MPI_COMM_WORLD);
	if (task_id == MASTER)
	{
		gettimeofday(&begin_3, 0);
	}
	if (task_id == MASTER)
	{
		qsort(splitter_buffer, splitter_buffer_size, sizeof(int), cmpfunc);

		for (int i = 1; i < num_tasks; i++)
		{
			global_splitter[i - 1] = splitter_buffer[i * (num_tasks - 1)];
		}

		global_splitter[global_splitter_size - 1] = INT_MAX; // make sure last element is greater than any one for a guaranteed bucket allocation
	}
	if (task_id == MASTER)
	{
		gettimeofday(&end_3, 0);
	}
	// after sorting global splitter send it to processes
	MPI_Bcast(global_splitter, global_splitter_size, MPI_INT, MASTER, MPI_COMM_WORLD);
	if (task_id == MASTER)
	{
		gettimeofday(&begin_4, 0);
	}
	// assign sample vector elements into local buckets
	for (int i = 0; i < sample_vec_size; i++)
	{
		int count = 0;
		while (count < global_splitter_size)
		{
			if (sample_vec[i] < global_splitter[count])
			{
				local_buckets[count][local_bucket_sizes[count]] = sample_vec[i];
				local_bucket_sizes[count]++;
				count = global_splitter_size; // element found bucket
			}
			count++;
		}
	}
	if (task_id == MASTER)
	{
		gettimeofday(&end_4, 0);
	}
	//gathering local buckets and there sizes at root process
	for (int i = 0; i < global_splitter_size; i++)
	{
		MPI_Gather(&local_bucket_sizes[i], 1, MPI_INT, global_bucket_buffer_sizes, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
		if (task_id == MASTER)
		{
			for (int j = 1; j < global_splitter_size; j++)
			{
				displ[j] = displ[j - 1] + global_bucket_buffer_sizes[j - 1];
			}
		}
		MPI_Gatherv(local_buckets[i], local_bucket_sizes[i], MPI_INT,
					global_buckets[i], global_bucket_buffer_sizes, displ, MPI_INT, MASTER, MPI_COMM_WORLD);
		if (task_id == MASTER)
		{
			for (int j = 0; j < global_splitter_size; j++)
			{
				global_bucket_buffer_sizes[j] = 0;
				displ[j] = 0;
			}
		}
	}

	global_bucket_sizes = (int *)calloc(num_tasks, sizeof(int));

	MPI_Reduce(local_bucket_sizes, global_bucket_sizes, num_tasks, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	MPI_Bcast(global_bucket_sizes, num_tasks, MPI_INT, MASTER, MPI_COMM_WORLD);

	local_ordered_bucket = (int *)malloc(sizeof(int) * global_bucket_sizes[task_id]);

	if (task_id == MASTER)
	{
		for (int i = 1; i < num_tasks; i++)
		{
			MPI_Send(global_buckets[i], global_bucket_sizes[i], MPI_INT, i, i, MPI_COMM_WORLD);
		}

		gettimeofday(&begin_5, 0);

		qsort(global_buckets[MASTER], global_bucket_sizes[MASTER], sizeof(int), cmpfunc);

		gettimeofday(&end_5, 0);
	}
	if (task_id > MASTER)
	{
		MPI_Recv(local_ordered_bucket, global_bucket_sizes[task_id], MPI_INT, MASTER, task_id, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		qsort(local_ordered_bucket, global_bucket_sizes[task_id], sizeof(int), cmpfunc);

		MPI_Send(local_ordered_bucket, global_bucket_sizes[task_id], MPI_INT, MASTER, task_id, MPI_COMM_WORLD);
	}

	if (task_id == 0)
	{
		for (int i = 1; i < num_tasks; i++)
		{
			MPI_Recv(global_buckets[i], global_bucket_sizes[i], MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}

	if (task_id == MASTER)
	{
		int count = 0;
		gettimeofday(&end, 0);
		if (print)
		{
			for (int i = 0; i < num_tasks; i++)
			{
				for (int j = 0; j < global_bucket_sizes[i]; j++)
				{
					output[count + j] = global_buckets[i][j];
				}
				count += global_bucket_sizes[i];
			}

			printf("INICIAL VECTOR\n");
			for (int i = 0; i < size; i++)
			{
				printf("%d ", vector[i]);
			}
			printf("\n\n\n");

			printf("SORTED RESULT\n");
			for (int i = 0; i < size; i++)
			{
				printf("%d ", output[i]);
			}
			printf("\n\n\n");
			long seconds = end.tv_sec - begin.tv_sec;
			long microseconds = end.tv_usec - begin.tv_usec;
			double elapsed = seconds + microseconds * 1e-6;
			printf("Time with mpi comunications measured: %.3f seconds.\n", elapsed);

			seconds = end_2.tv_sec - begin_2.tv_sec;
			microseconds = end_2.tv_usec - begin_2.tv_usec;
			elapsed = seconds + microseconds * 1e-6;

			seconds = end_3.tv_sec - begin_3.tv_sec;
			microseconds = end_3.tv_usec - begin_3.tv_usec;
			elapsed += seconds + microseconds * 1e-6;

			seconds = end_4.tv_sec - begin_4.tv_sec;
			microseconds = end_4.tv_usec - begin_4.tv_usec;
			elapsed += seconds + microseconds * 1e-6;

			seconds = end_5.tv_sec - begin_5.tv_sec;
			microseconds = end_5.tv_usec - begin_5.tv_usec;
			elapsed += seconds + microseconds * 1e-6;
			printf("Time without mpi comunications measured: %.3f seconds.\n", elapsed);
		}
	}
	MPI_Finalize();
	return 0;
}
