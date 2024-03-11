#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/select.h>

#define PORT 12000
#define MAX_CLIENTS 10

typedef struct {
    int client_id;
    int socket_fd;
} Client;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    Client *clients;
    pthread_t *threads;
    int socket;
} ThreadArgs;

int estabilish_connection(Client clients) {
    // TODO: Implementation of automatic connection stabilization between two clients
    return 0;
}

void *receive_send(void *arg) {
    ThreadArgs *args = (ThreadArgs*)arg;
    Client *clients = args->clients;
    pthread_t *threads = args->threads;
    int client_socket = args->socket;
    int read_size;
    int client_selected = -1;
    char buffer[1024];
    char select_client[1024];
    char *choose_client = "Which client are you want to connect?";

    while ((read_size = recv(client_socket, buffer, 1024, 0)) > 0) {
        buffer[read_size] = '\0';
        if (strncmp(buffer, "disconnect", strlen("disconnect")) == 0)  {
            disconnect_user(clients, client_socket);
            *threads = 0;
            break;
        }
        if (strncmp(buffer, "users", strlen("users")) == 0) {
            show_all_users(clients, client_socket);
        } 
        if (strncmp(buffer, "connect", strlen("connect")) == 0) {
            send(client_socket, choose_client, strlen(choose_client), 0);
            int size_test = recv(client_socket, select_client, 1024, 0);
            select_client[size_test] = '\0';
            client_selected = atoi(select_client);
            if (client_selected < 0) {
                printf("Client unavailable\n");
            }
            continue;
        }
        printf("Received message: %s", buffer);
        if (client_selected >= 0) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].client_id == client_selected) {
                    send(clients[i].socket_fd, buffer, strlen(buffer), 0);
                    break;
                }
            }
        }
    }
    if (read_size == 0 || read_size == -1) {
        disconnect_user(clients, client_socket);
        *threads = 0;
        pthread_exit(NULL);
    }
    return (void *)read_size;
}

int is_full(Client *clients) {
    return clients[MAX_CLIENTS].socket_fd != -1;
}

int find_index(Client *clients) {
    for (int i=0; i<MAX_CLIENTS; i++) {
        if (clients[i].socket_fd == -1) { // Each element of the clients[] is initialized to -1
            return i;
        }
    }
    printf("The server is full. Try again later");
    return -2;
}

int create_client(Client *clients, int socket_fd) {
    pthread_t client_threads[10];
    memset(client_threads, 0, sizeof(client_threads));
    int index;
    static int static_client_id = 0;

    if (is_full(clients)) {
        perror("Stack overflow\n");
        return -1;
    }
    pthread_mutex_lock(&mutex);
    index = find_index(clients);
    
    clients[index].client_id = static_client_id;
    clients[index].socket_fd = socket_fd;
    
    ThreadArgs args;
    args.clients = clients;
    args.socket = socket_fd;
    args.threads = &client_threads[index];
    pthread_create(&client_threads[index], NULL, receive_send, &args);
    pthread_mutex_unlock(&mutex);
    printf("Thread created successfully\n");
    static_client_id++;
    return 0;
}

int organize_list(Client *clients) {
    Client temp;
    pthread_mutex_lock(&mutex);
    for (int i=0; i<MAX_CLIENTS; i++) {
        if (clients[i].socket_fd == -1 && clients[i+1].socket_fd != -1) {
            temp = clients[i];
            clients[i] = clients[i+1];
            clients[i+1] = temp;
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int disconnect_user(Client *clients, int client_socket) {
    int on_list=0;
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd == client_socket) {
            clients[i].socket_fd = -1;
            clients[i].client_id = -1;
            on_list = 1;
            break;
        }
    }
    on_list ? printf("Client was disconnected successfully\n") : printf("The client was disconnected but was not found in the list\n");
    pthread_mutex_unlock(&mutex);
    close(client_socket);
    organize_list(clients);
    return 0;
}

void show_all_users(Client *clients, int client_socket) {
    char info[100];
    printf("Stack contents:\n");
    for (int i=0; i<MAX_CLIENTS; i++) {
        if (clients[i].socket_fd != -1 && clients[i].client_id != -1) {
            sprintf(info, "%d client with %d file descriptor", clients[i].client_id, clients[i].socket_fd);
            send(client_socket, info, 1024, 0);
        }
    }
}

int main() {
    int server_socket, client_socket, address_length;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;
    Client clients[MAX_CLIENTS+1]; // Get the size of these clients using MAX_CLIENTS.

    memset(clients,  -1, sizeof(clients));
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt error");
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
    fd_set readfds;
    char buffer[1024];
    
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int max_fd = server_socket > STDIN_FILENO ? server_socket : STDIN_FILENO;

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("Select failed");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_socket, &readfds)) {
            client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&address_length);
            if (client_socket < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }
            printf("New connection, socket fd is %d, ip is: %s, port : %d\n",
                client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            create_client(clients, client_socket);
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, sizeof(buffer), stdin);
            if (strcmp(buffer, "shutdown\n") == 0) {
                printf("Shutting down the server...\n");
                break;
            }
        }
    }
    return 0;
}
