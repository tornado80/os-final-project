#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "./utils.h"
#include <string.h>
#include <errno.h>

struct thread_info {
    char slot;
    sem_t sem;
};

int waiting_list = 0;
char threads_status[MAX_CLIENTS] = {0};
pthread_mutex_t threads_status_mutex;
struct thread_info threads[MAX_CLIENTS];

struct clients_memory *memory_a;
struct server_memory *memory_b;

void * handle_client (void *);

int main () {
    int fd_a = shm_open("/shm_server_a", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(fd_a, sizeof(struct clients_memory));
    memory_a = mmap(NULL, sizeof(struct clients_memory), PROT_READ | PROT_WRITE, MAP_SHARED, fd_a, 0);
    
    int fd_b = shm_open("/shm_server_b", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_b == -1) {
        printf("%d", errno);
        error("shm_open failed");
    }
    if(ftruncate(fd_b, sizeof(struct server_memory)))
        error("ftruncate error");
    memory_b = mmap(NULL, sizeof(struct server_memory), PROT_READ | PROT_WRITE, MAP_SHARED, fd_b, 0);
    if (memory_b == MAP_FAILED)
        error("mmap error");
    
    sem_init(&memory_b->server_request, 1, 0);
    sem_init(&memory_b->server_response, 1, 0);
    sem_init(&memory_b->server_waiting, 1, 0);
    sem_init(&memory_b->server_waiting_response, 1, 0);
    sem_init(&memory_b->server_connection, 1, 1);

    
    pthread_t tid;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        sem_init(&memory_b->server_semaphores[i], 1, 0);
        sem_init(&memory_a->clients_semaphores[i], 1, 0);
        sem_init(&threads[i].sem, 0, 0);
        threads[i].slot = i;
        pthread_create(&tid, NULL, handle_client, &threads[i]);
    }

    clear_screen();
    printf("server is ready ...\n");
    
    pthread_create(&tid, NULL, show_clients, NULL);
    pthread_mutex_init(&clients_count_mutex, NULL);
    pthread_mutex_init(&threads_status_mutex, NULL);
    sem_init(&clients_count_semaphore, 0, 1);

    while(1) {
        // wait for a request
        sem_wait(&memory_b->server_request);

        // decide to accept or reject (place in waiting list)
        pthread_mutex_lock(&threads_status_mutex);

        int flag = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (threads_status[i] == 0) { // idle worker, accept
                memory_a->status = i;
                threads_status[i] = 1;
                flag = 1;
                sem_post(&threads[i].sem);
                increase();
                break;
            }
        }
        if (flag == 0) { // reject
            memory_a->status = WAITING_STATUS;
            waiting_list += 1;
        } 
        sem_post(&memory_b->server_response);

        pthread_mutex_unlock(&threads_status_mutex);
    }
}

void * handle_client(void * arg) {
    struct thread_info * me = arg;
    while (1) {
        // wake up by server
        sem_wait(&me->sem);
        
        // wait for client
        sem_wait(&memory_b->server_semaphores[me->slot]);

        // copy received data from server to client buffer
        strcpy(memory_a->clients_buffers[me->slot], memory_b->server_buffers[me->slot]);

        // notify client
        sem_post(&memory_a->clients_semaphores[me->slot]);

        // update status to idle
        pthread_mutex_lock(&threads_status_mutex);

        if (waiting_list == 0) {
            threads_status[me->slot] = 0;
            decrease();
        } else {
            threads_status[me->slot] = 1;
            memory_a->status_waiting = me->slot;
            sem_post(&me->sem);
            sem_post(&memory_b->server_waiting);
            sem_wait(&memory_b->server_waiting_response);
        }

        pthread_mutex_unlock(&threads_status_mutex);
    }
}