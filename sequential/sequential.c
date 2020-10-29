#include "sequential.h"

int cmpfunc(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int *init_vector(int size) {
    int count = 0;
    int *vector = (int *)calloc(size, sizeof(int));
    for (int i = size; i >= 0; i--) {
        vector[count++] = i;
    }

    return vector;
}

int main(int argc, char *argv[]) {
    int num_tasks,
        sample_vec_size,
        size;
    int opt;
    int ***local_buckets, **global_buckets;
    int *vector, **sample_vec, *splitter, *splitter_buffer, *output;
    int **local_bucket_sizes, **global_bucket_buffer_sizes, **global_bucket_sizes;
    int splitter_size, splitter_buffer_size;
    int print = FALSE;
    int vec_count = 0;
    int task = 0;
    int count = 0;

    struct timeval begin, end;

    while ((opt = getopt(argc, argv, "s:n:hp")) != -1) {
        switch (opt) {
            case 's':
                size = atoi(optarg);
                break;
            case 'n':
                num_tasks = atoi(optarg);
                break;
            case 'h':
                printf("Usage: %s -n (nº processes) -s (size input) -h (help) -p (print result) \n", argv[0]);

                exit(EXIT_SUCCESS);
                break;
            case 'p':
                print = TRUE;
                break;
            case '?':

                exit(EXIT_FAILURE);
                break;
            default:

                abort();
                break;
        }
    }
    vector = init_vector(size);

    gettimeofday(&begin, 0);
    // initiliazations
    output = (int *)calloc(size, sizeof(int));

    sample_vec_size = size / num_tasks;
    sample_vec = (int **)malloc(sizeof(int *) * num_tasks);
    for (int i = 0; i < num_tasks; i++) {
        sample_vec[i] = (int *)calloc(sample_vec_size, sizeof(int));
    }

    splitter_buffer_size = num_tasks * (num_tasks - 1);
    splitter_buffer = (int *)calloc(splitter_buffer_size, sizeof(int));

    splitter_size = num_tasks;
    splitter = (int *)calloc(splitter_size, sizeof(int));

    local_buckets = (int ***)malloc(sizeof(int **) * num_tasks);
    for (int i = 0; i < num_tasks; i++) {
        local_buckets[i] = (int **)malloc(sizeof(int *) * splitter_size);
        for (int j = 0; j < splitter_size; j++) {
            local_buckets[i][j] = (int *)malloc(sizeof(int) * 2 * size / splitter_size);
        }
    }

    local_bucket_sizes = (int **)malloc(sizeof(int *) * num_tasks);
    for (int i = 0; i < num_tasks; i++) {
        local_bucket_sizes[i] = (int *)calloc(splitter_size, sizeof(int));
    }

    global_buckets = (int **)malloc(sizeof(int *) * splitter_size);

    for (int i = 0; i < splitter_size; i++) {
        global_buckets[i] = (int *)malloc(sizeof(int) * 2 * size / splitter_size);
    }

    // creating samples for each task

    while (vec_count < size) {
        if (count < sample_vec_size) {
            sample_vec[task][count++] = vector[vec_count++];
        } else {
            count = 0;
            task++;
        }
    }

    for (int i = 0; i < num_tasks; i++) {
        qsort(sample_vec[i], sample_vec_size, sizeof(int), cmpfunc);
    }
    // for (int i = 0; i < num_tasks; i++)
    // {
    // 	printf("TASK %d\n", i);
    // 	for (int j = 0; j < sample_vec_size; j++)
    // 	{
    // 		printf("%d ", sample_vec[i][j]);
    // 	}
    // 	printf("\n\n");
    // }

    // creating local splitters in each process
    count = 0;
    for (int i = 0; i < num_tasks; i++) {
        for (int j = 0; j < num_tasks - 1; j++) {
            splitter_buffer[count++] = sample_vec[i][size / (num_tasks * num_tasks) * (j + 1)];
        }
    }
    qsort(splitter_buffer, splitter_buffer_size, sizeof(int), cmpfunc);

    // for (int i = 0; i < splitter_buffer_size; i++)
    // {

    // 	printf("%d ", splitter_buffer[i]);
    // }
    // printf("\n");

    // // gathering each local splitter into a global one

    for (int i = 1; i < num_tasks; i++) {
        splitter[i - 1] = splitter_buffer[i * (num_tasks - 1)];
    }

    splitter[splitter_size - 1] = INT_MAX;  // make sure last element is greater than any one for a guaranteed bucket allocation

    // for (int i = 0; i < splitter_size; i++)
    // {

    // 	printf("%d ", splitter[i]);
    // }
    // printf("\n");

    // // after sorting global splitter send it to processes

    // assign sample vector elements into local buckets
    for (int j = 0; j < num_tasks; j++) {
        for (int i = 0; i < sample_vec_size; i++) {
            int count = 0;
            while (count < splitter_size) {
                if (sample_vec[j][i] < splitter[count]) {
                    local_buckets[j][count][local_bucket_sizes[j][count]] = sample_vec[j][i];
                    local_bucket_sizes[j][count]++;
                    count = splitter_size;  // element found bucket
                }
                count++;
            }
        }
    }

    // for (int i = 0; i < num_tasks; i++)
    // {
    // 	printf("TASK %d\n", i);
    // 	for (int j = 0; j < num_tasks; j++)
    // 	{
    // 		printf("BUCKET %d\n", j);
    // 		for (int k = 0; k < local_bucket_sizes[i][j]; k++)
    // 		{
    // 			printf("%d ", local_buckets[i][j][k]);
    // 		}

    // 		printf("\n");
    // 	}
    // 	printf("\n");
    // }

    // gathering local buckets and there sizes at root process
    int *count_bucket = (int *)calloc(num_tasks, sizeof(int));
    for (int i = 0; i < num_tasks; i++) {
        for (int j = 0; j < num_tasks; j++) {
            for (int k = 0; k < local_bucket_sizes[i][j]; k++) {
                global_buckets[j][count_bucket[j]++] = local_buckets[i][j][k];
            }
        }
    }

    // printf("FINAL BUCKET\n");
    // for (int i = 0; i < num_tasks; i++)
    // {
    // 	printf("BUCKET %d\n", i);
    // 	for (int j = 0; j < count_bucket[i]; j++)
    // 	{
    // 		printf("%d ", global_buckets[i][j]);
    // 	}
    // 	printf("\n");
    // }

    count = 0;
    for (int i = 0; i < num_tasks; i++) {
        qsort(global_buckets[i], count_bucket[i], sizeof(int), cmpfunc);
    }
    gettimeofday(&end, 0);
    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    printf("Time measured: %.3f seconds.\n", elapsed);

    if (print) {
        for (int i = 0; i < num_tasks; i++) {
            for (int j = 0; j < count_bucket[i]; j++) {
                output[count + j] = global_buckets[i][j];
            }
            count += count_bucket[i];
        }

        printf("INICIAL VECTOR\n");
        for (int i = 0; i < size; i++) {
            printf("%d ", vector[i]);
        }
        printf("\n\n\n");

        printf("SORTED RESULT\n");
        for (int i = 0; i < size; i++) {
            printf("%d ", output[i]);
        }
        printf("\n\n\n");
    }
}