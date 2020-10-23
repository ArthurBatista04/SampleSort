#include "samplesort.h"

int main(int argc, char *argv[])
{
	int num_tasks,
		task_id,
		num_workers,
		size_proc_input_vec,
		size;
	int opt, rc;
	int *vector, *process_input_vec;
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
		vector = (int *)calloc(size, sizeof(int));
		srand48((unsigned int)size);
		printf("\n---------------INICIAL VECTOR-----------------\n");
		for (int i = 0; i < size; i++)
		{
			vector[i] = rand();
			printf("%d ", vector[i]);
		}
		printf("\n\n");
	}

	MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	size_proc_input_vec = size / num_tasks;
	process_input_vec = (int *)calloc(size_proc_input_vec, sizeof(int));

	MPI_Scatter(vector, size_proc_input_vec, MPI_INT, process_input_vec,
				size_proc_input_vec, MPI_INT, MASTER, MPI_COMM_WORLD);

	printf("PROCESS %d SUB-VECTOR: ", task_id);
	for (int i = 0; i < size_proc_input_vec; i++)
	{
		printf("%d ", process_input_vec[i]);
	}
	printf("\n");

	MPI_Finalize();
	return 0;
}
