#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 12000
#define MAX_CLIENTS 10

typedef struct {
    int items[MAX_CLIENTS];
    int top;
} Stack;

Stack stack = { .items = {0} };

void initialize_stack(Stack *stack) {
    stack->top = -1;
}

int is_empty(Stack *stack) {
    return stack->top == -1;
}

int is_full(Stack *stack) {
    return stack->top == MAX_CLIENTS - 1;
}

void push(Stack *stack, int items) {
    if (is_full(&stack)) {
        printf("Stack overflow");
        return;
    }
    stack->items[++stack->top] = items;
}

int pop(Stack *stack) {
    int pop_value;
    if (is_empty) {
        printf("Stack underflow");
        return -1; // Error value
    }
    pop_value = stack->items[stack->top];
    stack->items[stack->top--] = 0;
    return pop_value;
}

void available_indeces(Stack *stack) {
    if (isEmpty(stack)) {
        printf("All indices are avaiable\n");
        return;
    }
    printf("Stack contents:\n");
    for (int i = 0; i < 13; i++) {
        if (stack->items[i] == 0) {
            printf("%d is available\n", i);
        }
    }
}


typedef struct {
    int socket_fd;
    int client_id;
} Client;

Client clients[MAX_CLIENTS];
pthread_t client_threads[MAX_CLIENTS];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *receive_send(void *arg) {
    int client_socket = *(int *)arg;
    int read_size, client_selected;
    char buffer[1024];
    char select_client;

    while ((read_size = recv(client_socket, buffer, 1024, 0)) > 0) {
        buffer[read_size] = '\0';
        if (strncmp(buffer, "disconnect", strlen("disconnect")) == 0)  {
            disconnect_user(client_socket);
            break;
        }
        if (strncmp(buffer, "users", strlen("users")) == 0) {
            show_all_users(client_socket);
        } 
        else if (strncmp(buffer, "connect", strlen("connect"))) {
            printf("Enter the client you want to connect: ");
            fgets(select_client, sizeof(select_client), stdin);
            // TODO: Implement client selection to chat
            client_selected = atoi(select_client);
            if (client_selected == -1) {
                printf("Client unavailable");
            }
        }

        printf("Received message: %s", buffer);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket_fd == client_socket) {
                send(clients[i].socket_fd, buffer, strlen(buffer), 0);
                break;
            }
            else if (clients[i].socket_fd == 0) {
                return 0;
            }
        }
    }
    if (read_size == 0 || read_size == -1) {
        disconnect_user(client_socket);
    }

    return (void *)read_size;
}

int disconnect_user(int client_socket) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket_fd == client_socket) {
            clients[i].socket_fd = -1;
            clients[i].client_id = -1;
            pthread_mutex_unlock(&mutex);
            printf("Client was disconnected successfully\n");
            close(client_socket);
            return 0;
        }
    }
    close(client_socket);
    printf("The client was disconnected but was not found in the list\n");
    return 0;
}

int create_client(int socket_fd) {
    static int next_client_id = 0;

    pthread_mutex_lock(&mutex);
    clients[next_client_id].client_id = next_client_id;
    clients[next_client_id].socket_fd = socket_fd;

    pthread_create(&client_threads[next_client_id], NULL, receive_send, &clients[next_client_id].socket_fd);
    next_client_id++;
    
    pthread_mutex_unlock(&mutex);
    printf("Client added at list\n");
    return next_client_id - 1;
}

int show_all_users(int client_socket) {
    int size = sizeof(clients)/sizeof(clients[0]);
    char info[100];
    for (int i=0; i<size; i++) {
        if (clients[i].socket_fd != 0 && clients[i].client_id != 0) {
            sprintf(info, "%d client with %d file descriptor", clients[i].client_id, clients[i].socket_fd);
            send(client_socket, info, 1024, 0);
        }
    }
    return 0;
}

int main() {
    int server_socket, client_socket, address_length;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;

    memset(clients, -1, sizeof(clients));
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
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&address_length))) {
        if (client_socket < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        printf("New connection, socket fd is %d, ip is: %s, port : %d\n",
               client_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        create_client(client_socket);
    }
    return 0;
}
