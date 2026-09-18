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
        printf("\nClient: %s", buffer);
        printf("You: ");
        fflush(stdout);
    }

    if (bytes_read == 0) {
        printf("\nClient disconnected.\n");
    } else {
        perror("Read error");
    }
    exit(0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    pthread_t recv_thread;
    char buffer[BUFFER_SIZE];

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to reuse address/port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Bind to localhost/any IP
    address.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected! You can start typing messages.\n");

    // Start thread to listen for incoming messages
    pthread_create(&recv_thread, NULL, receive_messages, &client_fd);

    // Main thread handles sending messages
    while (1) {
        printf("You: ");
        fflush(stdout);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            send(client_fd, buffer, strlen(buffer), 0);
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
