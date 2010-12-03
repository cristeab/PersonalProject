/*
 * player.c
 *
 *  Created on: Dec 2, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>

struct queue
{
	char *msg;
	void *next;
};
struct queue *queue_start = NULL;
struct queue *queue_current = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#define SRC_FILE "player.c"

int setup(FILE **stream)
{
	if (NULL == (*stream = fopen(SRC_FILE, "r")))
	{
		perror("fopen");
		return EXIT_FAILURE;
	}
	rewind(*stream);

	return EXIT_SUCCESS;
}

int read_data(FILE *stream)
{
	size_t line_len = 0;

	printf("entering into read_data\n");

	if (NULL == stream)
	{
		fprintf(stderr, "NULL stream\n");
		return EXIT_FAILURE;
	}

	while (!feof(stream))
	{
		if (0 != pthread_mutex_lock(&mutex))
		{
			perror("pthread_mutex_lock");
			return EXIT_FAILURE;
		}
		if (NULL == queue_current)
		{
			if (NULL == (queue_current = malloc(sizeof(struct queue))))
			{
				perror("malloc");
				return EXIT_FAILURE;
			}
			queue_current->msg = NULL;
			queue_current->next = NULL;
			queue_start = queue_current;
		}
		else
		{
			if (NULL == (queue_current->next = malloc(sizeof(struct queue))))
			{
				perror("malloc");
				return EXIT_FAILURE;
			}
			queue_current = queue_current->next;
			queue_current->msg = NULL;
			queue_current->next = NULL;
		}
		errno = 0;
		getline(&queue_current->msg, &line_len, stream);
		if (0 != errno)
		{
			perror("getline");
			return EXIT_FAILURE;
		}
		if (0 != pthread_mutex_unlock(&mutex))
		{
			perror("pthread_mutex_unlock");
			return EXIT_FAILURE;
		}
		if (0 != pthread_cond_signal(&cond))
		{
			perror("pthread_cond_signal");
			return EXIT_FAILURE;
		}
	}
	queue_current->next = (void*)-1;
	queue_current = queue_current->next;
	return (0 == fclose(stream))?EXIT_SUCCESS:EXIT_FAILURE;
}

void* process_data(void *data)
{
	struct queue *queue_release = NULL;

	printf("entering into process_data\n");

	while ((void*)-1 != queue_start)
	{
		if (0 != pthread_mutex_lock(&mutex))
		{
			perror("pthread_mutex_lock");
			return NULL;
		}
		while(NULL == queue_start)
		{
			if (0 != pthread_cond_wait(&cond, &mutex))
			{
				perror("pthread_cond_wait");
				return NULL;
			}
		}
		queue_release = queue_start;
		queue_start = queue_start->next;
		if (queue_release == queue_current)
		{
			queue_current = NULL;
		}
		if (0 != pthread_mutex_unlock(&mutex))
		{
			perror("pthread_mutex_unlock");
			return NULL;
		}
		printf("%s", queue_release->msg);
		fflush(stdout);
		free(queue_release->msg);
		free(queue_release);
	}
	return NULL;
}

int main()
{
	pthread_t tid;
	FILE *stream = NULL;
	void *ret_val = NULL;

	if (EXIT_SUCCESS != setup(&stream))
	{
		return EXIT_FAILURE;
	}

	if (0 != pthread_create(&tid, NULL, process_data, NULL))
	{
		perror("pthread_create");
		return EXIT_FAILURE;
	}

	if (EXIT_SUCCESS != read_data(stream))
	{
		return EXIT_FAILURE;
	}

	if (0 != pthread_join(tid, &ret_val))
	{
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
