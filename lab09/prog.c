#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

#define MAX_THREAD_COUNT 30000

typedef struct sum_args {
	int thread_count;
	int offset;
} sum_args;

typedef double res_t;

int is_running = 1;
static pthread_barrier_t barrier;

void handle_int(int signum) {
	if(signum == SIGINT) {
		is_running = 0;
	}
}

void *partical_sum(void *args) {
	sum_args *params = (sum_args*) args;
	int thread_count = params->thread_count;
	int offset = params->offset;
    
	res_t* part_sum = calloc(1, sizeof(res_t));
	const int block_size = 10000;
	for(long long block_num = 0; is_running; block_num++) {
		for(long long iteration = 0; iteration < block_size; iteration++) {
			long long it = block_size * block_num + iteration;
			long long member = (it * thread_count + offset) * 4 + 1;
			*part_sum += 1. / member - 1. / (member + 2.);
		}
		pthread_barrier_wait(&barrier);
	}
	pthread_exit((void*) part_sum);
}

int main(int argc, char **argv) {
	signal(SIGINT, handle_int);
	if(argc != 2) {
		fprintf(stderr, "Wrong argc\n");
		pthread_exit(NULL);
	}
	int thread_count = atoi(argv[1]);
	if(thread_count > MAX_THREAD_COUNT) {
		thread_count = MAX_THREAD_COUNT;
		fprintf(stderr, "thread count decreased to %d\n", thread_count);
	}

	pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) *
			thread_count);

	if(threads == NULL) {
		perror("thread_t malloc");
		pthread_exit(NULL);
	}

	sum_args *sargs = (sum_args*) malloc(sizeof(sum_args) * thread_count);

	if(sargs == NULL) {
		perror("sargs malloc");
		free(threads);
		pthread_exit(NULL);
	}

	if(pthread_barrier_init(&barrier, NULL, thread_count) < 0) {
		perror("barrier init");
		free(threads);
		free(sargs);
		pthread_exit(NULL);
	}

	for(int index = 0; index < thread_count; index++) {
		sargs[index].thread_count = thread_count;
		sargs[index].offset = index;
	}

	int true_count = thread_count;
	for(int index = 0; index < thread_count; index++) {
		if(pthread_create(threads + index, NULL, partical_sum,
				sargs + index) != 0) {
			perror("couldn't create a new thread");
			printf("created %d threads instead of %d\n", index,
					thread_count);
			true_count = index;
			break;
		}
	}

	res_t result = 0;
	for(int index = 0; index < true_count; index++) {
		res_t *part_sum;
		if(pthread_join(threads[index], (void**) &part_sum) != 0) {
			perror("pthread join");
			printf("thread #%d\n", index);
		}
		else {
			result += *part_sum;
			free(part_sum);
		}
	}
	result *= 4;
	printf("\nPi=\t%.15f\n", result);
	printf("Pi_0=\t%.15f\n", M_PI);
	printf("d=\t%.15f\n", M_PI - result);

	free(threads);
	free(sargs);
	pthread_barrier_destroy(&barrier);
	pthread_exit(NULL);
}

