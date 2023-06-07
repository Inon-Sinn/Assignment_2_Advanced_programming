#include "codec.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

typedef struct Job{
    char *chunk;
    int index;
    struct Job *next;
} Job;

typedef struct {
    Job *jobs;
    Job *last;
    int current_chunk;
    int encrypt;
    int key;
    char* result;
    pthread_mutex_t joblist;
    pthread_mutex_t res_update;
    pthread_cond_t next_chunk;
} ThreadPool;


void* thread_function(void* arg) {
    ThreadPool* pool = (ThreadPool*)arg;

    while (1) {
        // lock job list and search for job
        pthread_mutex_lock(&(pool->joblist));
        if (pool -> jobs == NULL){
            pthread_mutex_unlock(&(pool->joblist));
            break; // all the work was assigned
        }
        // Take first job
        Job* job = pool->jobs;
        pool->jobs = job->next;
        pthread_mutex_unlock(&(pool->joblist));

        // de/encrypt according to pool data
        if(pool->encrypt){
            // encrypt data according to key
            encrypt(job->chunk, pool->key);
        }
        else
        {
            decrypt(job->chunk, pool->key);
        }

        // wait for your turn, then concatenate result
        while (pool->current_chunk != job->index-1) {
            // previous chunk not finished
            pthread_cond_wait(&(pool->next_chunk), &(pool->res_update));
        }
        // previous chunk finished
        // Synchronize and reassemble the data to 1 long string
        // add space to result
        char* new_res = malloc((strlen(pool->result)+strlen(job->chunk)+1)*sizeof(char));
        memcpy(new_res, pool->result, strlen(pool->result)+1);
        free(pool->result);
        pool->result = new_res;
        // append current chunk to result
        strcat(pool->result, job->chunk);
        free(job->chunk);
        free(job);
        pool->current_chunk++;
        pthread_cond_broadcast(&(pool->next_chunk));
        pthread_mutex_unlock(&(pool->res_update));
    }

    pthread_exit(NULL);
}

void init_thread_pool(ThreadPool* pool,int encrypt, int key) {
    pool->jobs = NULL;
    pool->last = NULL;
    pool->current_chunk = -1;
    pool->encrypt = encrypt;
    pool->key = key;
    pool->result = (char*)malloc(sizeof(char));
    pool->result[0]='\0';
    pthread_mutex_init(&(pool->joblist), NULL);
    pthread_mutex_init(&(pool->res_update), NULL);
    pthread_cond_init(&(pool->next_chunk), NULL);
}

void destroy_thread_pool(ThreadPool* pool) {
    free(pool->result);
    pthread_mutex_destroy(&(pool->joblist));
    pthread_mutex_destroy(&(pool->res_update));
    pthread_cond_destroy(&(pool->next_chunk));
}



int main(int argc, char *argv[]) {
if (argc != 3)
	{
	    printf("usage: key flag(-e/-d)\n");
	    return 0;
	}

    // bool value for encrypt-1 or decrypt-0
    int flag=0;

    if(strcmp(argv[2], "-d") == 0){
        flag = 0;
    }
    else if(strcmp(argv[2], "-e") == 0){
        flag = 1;
    }
    else{
        printf("usage: key flag < file \n");
        printf("No such flag, availabe flags: encrpyt '-e', decrypt '-d'\n");
        return 0;
    }
	int key = atoi(argv[1]);

    ThreadPool pool_data;
    ThreadPool* pool = &pool_data;
    init_thread_pool(pool, flag, key);

    // recieve input and split to chunks, store in list. 
    char buffer[BUFFER_SIZE];
    int index = 0;
    int chunk_index = 0;
    int ch;

    while ((ch = getchar()) != EOF) {
        buffer[index] = ch;
        index++;

        if (index == BUFFER_SIZE) {
            char* chunk = (char*)malloc(BUFFER_SIZE*sizeof(char)+1);
            Job* job = (Job*)malloc(sizeof(Job));
            if(chunk == NULL || job == NULL){
                printf("Memory not allocated.\n");
                exit(0);
            }
            memcpy(chunk, buffer, BUFFER_SIZE);
            chunk[BUFFER_SIZE] = '\0';
            job->chunk = chunk;
            job->index = chunk_index;
            
            // add job to queue
            if(pool -> last == NULL){
                // add first job
                pool -> jobs = job;
                pool -> last = job;
            }
            else
            {
                pool->last->next = job;
                pool->last = job;
            }
            chunk_index++;
            index = 0;
        }
    }

    if (index > 0) {
        // add last chunk
        char* chunk = malloc(index*sizeof(char)+1);
        Job* job = (Job*)malloc(sizeof(Job));
        if(chunk == NULL || job == NULL){
            printf("Memory not allocated.\n");
            exit(0);
        }
        memcpy(chunk, buffer, index);
        chunk[index] = '\0';
        job->chunk = chunk;
        job->index = chunk_index;
        // add job to queue
        if(pool -> last == NULL){
            // add first job
            pool -> jobs = job;
            pool -> last = job;
        }
        else
        {
            pool->last->next = job;
            pool->last = job;
        }
    }

    // create threads and let them run on the joblist
    int numCores = sysconf(_SC_NPROCESSORS_ONLN);

    pthread_t threads[numCores];
    int i;
    for (i = 0; i < numCores; i++) {
        pthread_create(&(threads[i]), NULL, thread_function, (void*)pool);
    }

    for (i = 0; i < numCores; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%s", pool->result);

    destroy_thread_pool(pool);

    return 0;
}
