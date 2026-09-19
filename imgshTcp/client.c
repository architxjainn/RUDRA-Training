#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <sys/stat.h>

#define PORT 5001
#define BUFFER_SIZE 4096

int receive_exact(int socket_fd, void *buffer, size_t length) {
    size_t total_received = 0;
    char *ptr = (char *)buffer;
    while (total_received < length) {
        ssize_t bytes_read = read(socket_fd, ptr + total_received, length - total_received);
        if (bytes_read <= 0) return -1;
        total_received += bytes_read;
    }
    return 0;
}

int send_image_turn(int socket_fd) {
    char filepath[512];
    FILE *fp = NULL;

    printf("\n[SEND TURN] Your turn to transmit an image.\n");

    while (1) {
        printf("Enter image path (or drag file here) > ");
        fflush(stdout);

        if (!fgets(filepath, sizeof(filepath), stdin)) return -1;
        filepath[strcspn(filepath, "\r\n")] = 0;

        char clean_path[512];
        int j = 0;
        for (int i = 0; filepath[i] != '\0'; i++) {
            if (filepath[i] != '\'' && filepath[i] != '"') {
                clean_path[j++] = filepath[i];
            }
        }
        clean_path[j] = '\0';

        if (strlen(clean_path) == 0) continue;

        fp = fopen(clean_path, "rb");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            uint64_t file_size = ftell(fp);
            fseek(fp, 0, SEEK_SET);

            uint64_t file_size_net = htobe64(file_size);
            if (write(socket_fd, &file_size_net, sizeof(file_size_net)) <= 0) {
                fclose(fp);
                return -1;
            }

            char buffer[BUFFER_SIZE];
            size_t bytes_read;
            while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
                if (write(socket_fd, buffer, bytes_read) <= 0) {
                    fclose(fp);
                    return -1;
                }
            }

            fclose(fp);
            printf("[✓] Sent '%s' (%llu bytes)\n", clean_path, (unsigned long long)file_size);
            break;
        } else {
            printf("[-] File not found. Try again.\n");
        }
    }
    return 0;
}

int receive_image_turn(int socket_fd, int image_counter) {
    printf("\n[RECEIVE TURN] Waiting for server to send an image...\n");

    uint64_t image_size_net;
    if (receive_exact(socket_fd, &image_size_net, sizeof(image_size_net)) < 0) {
        printf("[-] Connection lost.\n");
        return -1;
    }

    uint64_t image_size = be64toh(image_size_net);

    char filename[64];
    snprintf(filename, sizeof(filename), "client_received_%d.jpg", image_counter);

    FILE *fp = fopen(filename, "wb");
    if (!fp) return -1;

    char buffer[BUFFER_SIZE];
    size_t remaining = image_size;
    while (remaining > 0) {
        size_t to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
        ssize_t bytes_read = read(socket_fd, buffer, to_read);
        if (bytes_read <= 0) {
            fclose(fp);
            return -1;
        }
        fwrite(buffer, 1, bytes_read, fp);
        remaining -= bytes_read;
    }

    fclose(fp);
    printf("[✓] Received image saved as: %s (%llu bytes)\n", filename, (unsigned long long)image_size);

    char open_cmd[128];
    snprintf(open_cmd, sizeof(open_cmd), "open '%s'", filename);
    system(open_cmd);

    return 0;
}

int main() {
    int sock_fd;
    struct sockaddr_in serv_addr;
    char target_ip[64];

    printf("Enter Server IP Address (use '127.0.0.1' for local testing): ");
    if (!fgets(target_ip, sizeof(target_ip), stdin)) return 0;
    target_ip[strcspn(target_ip, "\r\n")] = 0;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, target_ip, &serv_addr.sin_addr) <= 0) {
        printf("Invalid IP address.\n");
        return -1;
    }

    printf("[+] Connecting to server at %s:%d...\n", target_ip, PORT);
    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    printf("[+] Connected to server!\n");

    int is_sender = 0; // Client receives first
    int count = 1;

    while (1) {
        if (is_sender) {
            if (send_image_turn(sock_fd) < 0) break;
            is_sender = 0;
        } else {
            if (receive_image_turn(sock_fd, count++) < 0) break;
            is_sender = 1;
        }
    }

    close(sock_fd);
    return 0;
}
