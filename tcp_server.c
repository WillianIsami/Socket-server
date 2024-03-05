#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 12000
#define MAX_CLIENTS 2

int clients[MAX_CLIENTS];
pthread_t client_threads[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *receive_send(void *arg) {
    int client_socket = *(int *)arg;
    int read_size;
    char buffer[1024];

    while ((read_size = recv(client_socket, buffer, 1024, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("Received message: %s", buffer);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != client_socket && clients[i] != 0) {
                send(clients[i], buffer, strlen(buffer), 0);
            }
        }
    }
    return (void *)read_size;
}

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    int size = sizeof(clients)/sizeof(clients[0]);
    int read_size;
    pthread_t thread[MAX_CLIENTS];

    for(int i=0; i<size; i++){
        pthread_create(&thread[i], NULL, receive_send, &clients[i]);
    }
    for(int i=0; i<size; i++) {
        pthread_join(thread[i], (void **)&read_size);
    }

    if (read_size == 0) {
        printf("Client disconnected\n");
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == client_socket) {
                clients[i] = 0;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
    } else if (read_size == -1) {
        perror("recv failed");
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket, address_length;
    struct sockaddr_in server_addr, client_addr;

    memset(clients, 0, sizeof(clients));
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    address_length = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&address_length))) {
        if (client_socket < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        printf("New connection, socket fd is %d, ip is: %s, port : %d\n",
               client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == 0) {
                clients[i] = client_socket;
                pthread_create(&client_threads[i], NULL, handle_client, &clients[i]);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
    }
    return 0;
}