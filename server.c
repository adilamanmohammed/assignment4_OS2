#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1051
#define MAX 3

FILE* s_file;
pthread_mutex_t mutex;


void* connection(void* input) {
    int c_sock = *((int*) input);
    char buff_var[1024];
    ssize_t bytes_recv = recv(c_sock, buff_var, 1024, 0);
    if (bytes_recv < 0) {
        close(c_sock);
        pthread_exit(NULL);
    }

    buff_var[bytes_recv] = '\0';

    pthread_mutex_lock(&mutex);
    fprintf(s_file, "%s\n", buff_var);
    fflush(s_file);
    pthread_mutex_unlock(&mutex);

    sleep(2);

    pthread_mutex_lock(&mutex);
    fseek(s_file, 0, SEEK_SET);
    while (fgets(buff_var, 1024, s_file) != NULL) {
        if (send(c_sock, buff_var, strlen(buff_var), 0) < 0) {
            close(c_sock);
            pthread_exit(NULL);
        }
    }
    pthread_mutex_unlock(&mutex);

    close(c_sock);
    pthread_exit(NULL);
}

int main() {
    int s_sock, c_sock;
    struct sockaddr_in s_addr, c_addr;
    socklen_t length;
    pthread_t threads[MAX];

    s_file = fopen("shared.txt", "a+");
    if (s_file == NULL) {
        perror("fopen error");
        exit(1);
    }

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("pthread_mutex_init error");
        exit(1);
    }

    s_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (s_sock < 0) {
        perror("socket error");
        exit(1);
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_port = htons(PORT);

    if (bind(s_sock, (struct sockaddr*) &s_addr, sizeof(s_addr)) < 0) {
        perror("binding error");
        exit(1);
    }

    if (listen(s_sock, MAX) < 0) {
        perror("listen error");
        exit(1);
    }

    printf("Server started:\n");
    for (int i = 0; i < MAX; i++) {
        printf("Waiting for connections to connect:\n");
        length = sizeof(c_addr);
        c_sock = accept(s_sock, (struct sockaddr*) &c_addr, &length);
        if (c_sock < 0) {
            perror("accept");
            exit(1);
        }
        printf("Connection from %s:%d Accepted \n", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
        if (pthread_create(&threads[i], NULL, connection, (void*) &c_sock) != 0) {
            perror("Thread creation error");
            exit(1);
        }
    }

    for (int i = 0; i < MAX; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Thread joining error");
            exit(1);
        }
    }

    fclose(s_file);
    pthread_mutex_destroy(&mutex);
    close(s_sock);
    exit(1);
}

