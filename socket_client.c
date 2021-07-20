#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>
#include"./utils.h"

int main(int argc, char* argv[]) {
    char *host, *text;
    int port;
    char* options[] = {"-h", "-p", "-t"};
    char* options_values[] = {NULL, NULL, NULL};
    for (int i = 1; i < argc; i++)
        for (int j = 0; j < 3; j++)
            if (strcmp(options[j], argv[i]) == 0 && i < (argc - 1))
                options_values[j] = argv[i++ + 1];
    if (options_values[0] == NULL || 
        options_values[1] == NULL || 
        options_values[2] == NULL)
        error("Wrong arguments\nusage: -h <host> -p <port> -t <text>");
    host = options_values[0];
    port = atoi(options_values[1]);
    text = options_values[2];
    struct sockaddr_in address = {AF_INET, htons(port), {inet_addr(host)}};
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        error("socket creation failed");
    if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1)
        error("socket connection failed");

    unsigned char buffer[8195] = {0};
    unsigned char recv_buffer[8193] = {0};
    unsigned short len = strlen(text);
    unsigned short recv_len;

    buffer[1] = len;
    buffer[0] = len >> 8;
    strcpy(buffer + 2, text);

    struct timespec begin, end;

    clock_gettime(CLOCK_REALTIME, &begin);

    sleep(4);

    send(sockfd, buffer, len + 2, 0);

    recv(sockfd, recv_buffer, 2, 0);
    recv_len = recv_buffer[1] + (recv_buffer[0] << 8);

    recv(sockfd, recv_buffer, recv_len, 0);

    clock_gettime(CLOCK_REALTIME, &end);

    int elapsed_seconds = end.tv_sec - begin.tv_sec;
    long elapsed_nanoseconds = end.tv_nsec - begin.tv_nsec;

    printf("recieved in %d.%ld seconds\nmessage: %s\n", elapsed_seconds, elapsed_nanoseconds, recv_buffer);
}