#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8081
#define MAX_THREADS 3
#define MAX_BUFFER_SIZE 1024

pthread_mutex_t mutex;
FILE* shared_file;

void* handle_connection(void* arg) {
    int client_socket = *((int*) arg);
    char buffer[MAX_BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, buffer, MAX_BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(client_socket);
        pthread_exit(NULL);
    }
    buffer[bytes_received] = '\0';

    pthread_mutex_lock(&mutex);
    fprintf(shared_file, "%s\n", buffer);
    fflush(shared_file);
    pthread_mutex_unlock(&mutex);

    sleep(2);

    pthread_mutex_lock(&mutex);
    fseek(shared_file, 0, SEEK_SET);
    while (fgets(buffer, MAX_BUFFER_SIZE, shared_file) != NULL) {
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            close(client_socket);
            pthread_mutex_unlock(&mutex);
            pthread_exit(NULL);
        }
    }
    pthread_mutex_unlock(&mutex);

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;
    pthread_t threads[MAX_THREADS];

    shared_file = fopen("shared.txt", "a+");
    if (shared_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_THREADS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        printf("Waiting for connection...\n");
        client_address_length = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length);
        if (client_socket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
        if (pthread_create(&threads[i], NULL, handle_connection, (void*) &client_socket) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    fclose(shared_file);
    pthread_mutex_destroy(&mutex);
    close(server_socket);
    exit(EXIT_SUCCESS);
}

