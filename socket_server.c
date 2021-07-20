#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<semaphore.h>
#include<errno.h>
#include"./utils.h"

struct peer_info {
    int sockfd;
    struct sockaddr_in address;
};

void * handle_client(void *);

int main(int argc, char* argv[]) {
    char *host;
    int port;
    char* options[] = {"-h", "-p"};
    char* options_values[] = {NULL, NULL};
    for (int i = 1; i < argc; i++)
        for (int j = 0; j < 2; j++)
            if (strcmp(options[j], argv[i]) == 0 && i < (argc - 1))
                options_values[j] = argv[i++ + 1];
    if (options_values[0] == NULL || options_values[1] == NULL)
        error("Wrong arguments\nusage: -h <host> -p <port>");
    host = options_values[0];
    port = atoi(options_values[1]);
    struct sockaddr_in address = {AF_INET, htons(port), {inet_addr(host)}};
    int addrlen = sizeof(address);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        error("socket creation failed");
    if (bind(sockfd, (struct sockaddr *)(&address), sizeof(address)) == -1)
        error("socket binding failed");
    if (listen(sockfd, -1) == -1)
        error("socket listening failed");
    clear_screen();
    printf("server is ready (listening on %s:%d) ...\n", host, port);
    pthread_t tid;
    sem_init(&clients_count_semaphore, 0, 1);
    if(pthread_create(&tid, NULL, show_clients, NULL) != 0)
        error("can not create a thread");
    pthread_mutex_init(&clients_count_mutex, NULL);
    while (1) {
        struct peer_info * peer = malloc(sizeof(struct peer_info));
        int peer_sockfd = accept(sockfd, (struct sockaddr *)(&peer->address), (socklen_t *)(&addrlen));
        if (peer_sockfd == -1)
            error("socket accepting connections failed");
        peer->sockfd = peer_sockfd;
        increase();
        if(pthread_create(&tid, NULL, handle_client, peer) != 0)
            error("can not create a thread");
    }
}

void * handle_client(void * arg) {
    struct peer_info * peer = arg;
    short bytes_read = 0;
    unsigned char buffer[8194] = {0};
    unsigned short len;

    recv(peer->sockfd, buffer, 2, 0);
    len = buffer[1] + (buffer[0] << 8);

    recv(peer->sockfd, buffer + 2, len, 0);

    buffer[1] = len;
    buffer[0] = len >> 8;

    send(peer->sockfd, buffer, len + 2, 0);

    decrease();
    free(peer);
    return NULL;
}