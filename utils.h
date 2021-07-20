#include<semaphore.h>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>

#define ANSI_ERASE_LINE "\e[K"
#define ANSI_SAVE_CURSOR "\e[s"
#define ANSI_RESTORE_CURSOR "\e[u"

#define WAITING_STATUS -1
#define MAX_CLIENTS 8

struct clients_memory {
    int status;
    int status_waiting;
    char clients_buffers[MAX_CLIENTS][8193];
    sem_t clients_semaphores[MAX_CLIENTS];
};

struct server_memory {
    sem_t server_connection;
    sem_t server_request;
    sem_t server_response;
    sem_t server_waiting;
    sem_t server_waiting_response;
    char server_buffers[MAX_CLIENTS][8193];
    sem_t server_semaphores[MAX_CLIENTS];
};

void error(const char* msg);

void clear_screen ();

void erase_current_line ();

void * show_clients(void *);
void increase();
void decrease();

extern sem_t clients_count_semaphore;
extern int clients_count;
extern pthread_mutex_t clients_count_mutex;