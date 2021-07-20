#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "./utils.h"
#include <string.h>
#include <time.h>

int main (int argc, char * argv []) {
    int fd_a = shm_open("/shm_server_a", O_RDWR, 0);
    struct clients_memory *memory_a = mmap(NULL, sizeof(struct clients_memory),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd_a, 0);
    int fd_b = shm_open("/shm_server_b", O_RDWR, 0);
    struct server_memory *memory_b = mmap(NULL, sizeof(struct server_memory),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd_b, 0);

    int slot = -1;

    // establish a connection
    sem_wait(&memory_b->server_connection);

    // send request
    sem_post(&memory_b->server_request);

    // wait for server decision
    sem_wait(&memory_b->server_response);

    // check server decison
    slot = memory_a->status;

    // close connection
    sem_post(&memory_b->server_connection);

    // waiting list
    if (slot == WAITING_STATUS) {
        sem_wait(&memory_b->server_waiting);
        slot = memory_a->status_waiting;
        sem_post(&memory_b->server_waiting_response);
    }
        
    // do communication
    struct timespec begin, end;
    clock_gettime(CLOCK_REALTIME, &begin);

    strcpy(memory_b->server_buffers[slot], argv[1]);
    sem_post(&memory_b->server_semaphores[slot]);
    sem_wait(&memory_a->clients_semaphores[slot]);

    clock_gettime(CLOCK_REALTIME, &end);
    int elapsed_seconds = end.tv_sec - begin.tv_sec;
    long elapsed_nanoseconds = end.tv_nsec - begin.tv_nsec;
    printf("recieved in %d.%ld seconds\nmessage: %s\n", elapsed_seconds, elapsed_nanoseconds, memory_a->clients_buffers[slot]);
}