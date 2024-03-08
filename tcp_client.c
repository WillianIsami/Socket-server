#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 12000
#define IP "127.0.0.1"

void *receive_and_print(void *arg) {
    int client_socket = *(int *)arg;
    int read_recv;
    char message[1024];

    while ((read_recv = recv(client_socket, message, 1024, 0)) > 0) {
        printf("\nResponse: %s", message);
        memset(message, 0, sizeof(message));
    }
    pthread_exit(NULL);
}

int main() {
    int client_socket;
    char message[1024];
    struct sockaddr_in server_addr;
    pthread_t recv_thread;

    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        exit(EXIT_FAILURE);
    }
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to server\n"
            "If you want to disconnect then press 'disconnect'\n");
    pthread_create(&recv_thread, NULL, receive_and_print, &client_socket);

    while (1) {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);
        send(client_socket, message, strlen(message), 0);
        if (strncmp(message, "disconnect", 10) == 0) {
            close(client_socket);
            printf("Disconnected successfully.\n");
            return;
        }   
    }
    pthread_join(recv_thread, NULL);
    close(client_socket);
    return 0;
}
