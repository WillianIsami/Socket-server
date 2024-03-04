#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int createSocket() {
    int fsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(fsocket == -1){
        printf("Error: Could not create socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Socket created sucessfull\n");
    return fsocket;
}

int bindSocket(int sockfd) {
    int port = 8080;
    struct sockaddr_in remote;

    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = INADDR_ANY;
    remote.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
        perror("Bind failed\n");
        exit(EXIT_FAILURE);
    }

    printf("Bind successful\n");
    return 0;
}

int socketConnection(int socket_desc) {
    int sock, clientLen;
    struct sockaddr_in client;
    char client_message[200];
    char message[100];
    const char *nMessage = "new Testing";

    listen(socket_desc, 3);
    while(1) {
        printf("Waiting for incoming connections...\n");
        clientLen = sizeof(struct sockaddr_in);

        sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&clientLen);
        if (sock < 0) {
            perror("accept failed");
            return 1;
        }
        printf("Connection accepted\n");
        memset(client_message, '\0', sizeof client_message);
        memset(message, '\0', sizeof message);
        
        if (recv(sock, client_message, 200, 0) < 0) {
            printf("recv failed");
            break;
        }
        printf("Client reply : %s\n",client_message);
        if (strcmp(nMessage, client_message)==0) {
            strcpy(message,"Hello! Testing");
        } else {
            strcpy(message,"Invalid Message !");
        }
        if (send(sock, message, strlen(message), 0) < 0) {
            printf("Send failed");
            return 1;
        }
        close(sock);
        sleep(1);
    }
    return 0;
}

int main() {
    int sockfd;
    printf("Offdout server \n");

    sockfd = createSocket();
    bindSocket(sockfd);
    socketConnection(sockfd);
    return 0;
}