#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Thread function to handle incoming messages
void *receive_messages(void *socket_fd_ptr) {
    int socket_fd = *(int *)socket_fd_ptr;
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while ((bytes_read = read(socket_fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("\nServer: %s", buffer);
        printf("You: ");
        fflush(stdout);
    }

    if (bytes_read == 0) {
        printf("\nServer disconnected.\n");
    } else {
        perror("Read error");
    }
    exit(0);
}

int main() {
    int sock_fd = 0;
    struct sockaddr_in serv_addr;
    pthread_t recv_thread;
    char buffer[BUFFER_SIZE];

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address / Address not supported \n");
        return -1;
    }

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("Connected to server! You can start typing messages.\n");

    // Start thread to listen for incoming messages
    pthread_create(&recv_thread, NULL, receive_messages, &sock_fd);

    // Main thread handles sending messages
    while (1) {
        printf("You: ");
        fflush(stdout);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            send(sock_fd, buffer, strlen(buffer), 0);
        }
    }

    close(sock_fd);
    return 0;
}
