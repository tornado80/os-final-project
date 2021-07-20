#include"./utils.h"

sem_t clients_count_semaphore;
int clients_count = 0;
pthread_mutex_t clients_count_mutex;

void error(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

void clear_screen () {
    system("clear");
}

void erase_current_line () {
    printf("\r");
    printf(ANSI_SAVE_CURSOR);
    printf(ANSI_ERASE_LINE);
    printf(ANSI_RESTORE_CURSOR);
    fflush(stdout);
}

void increase() {
    pthread_mutex_lock(&clients_count_mutex);
    clients_count++;
    pthread_mutex_unlock(&clients_count_mutex);
    sem_post(&clients_count_semaphore);
}

void decrease() {
    pthread_mutex_lock(&clients_count_mutex);
    clients_count -= 1;
    pthread_mutex_unlock(&clients_count_mutex);
    sem_post(&clients_count_semaphore);
}

void * show_clients(void * arg) {
    while(1) {
        sem_wait(&clients_count_semaphore);
        pthread_mutex_lock(&clients_count_mutex);
        int x = clients_count;
        pthread_mutex_unlock(&clients_count_mutex);
        erase_current_line();
        printf("connected clients: %d", x);
        fflush(stdout);
    }
}