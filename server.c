#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT 20000
#define BUFFER_SIZE 1024
#define SERVER_DIR "server_dir"

void send_file(int client_socket, const char *filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", SERVER_DIR, filename);

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("File open failed");
        const char *error_message = "File not found on the server.\n";
        send(client_socket, error_message, strlen(error_message), 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    printf("File sent successfully: %s\n", filename);
    fclose(file);

    close(client_socket);
}

void receive_file(int client_socket, const char *filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", "server_dir", filename);

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        perror("File open failed");
        return;
    }

    printf("Receiving file: %s\n", filename);  // Debugging line

    char buffer[BUFFER_SIZE];
    int bytes_received;
    int total_bytes_received = 0;

    // Receive data in chunks
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        printf("Received %d bytes\n", bytes_received);  // Debugging line

        // Write data to the file
        if (fwrite(buffer, 1, bytes_received, file) != bytes_received) {
            perror("File write failed");
            fclose(file);
            return;
        }

        total_bytes_received += bytes_received;
    }

    printf("Total bytes received: %d\n", total_bytes_received);  // Debugging line
    printf("File received successfully: %s\n", filename);
    fclose(file);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    char command[BUFFER_SIZE];
    while (1) {
        memset(command, 0, sizeof(command));

        // Receive the command (e.g., "put filename.txt" or "get filename.txt")
        int bytes_received = recv(client_socket, command, sizeof(command) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }

        command[bytes_received] = '\0';  // Null-terminate the string
        printf("Received command: %s\n", command);

        if (strncmp(command, "put ", 4) == 0) {
            // Extract the filename and receive the file
            char filename[256];
            sscanf(command + 4, "%s", filename);
            receive_file(client_socket, filename);
        } else if (strncmp(command, "get ", 4) == 0) {
            // Extract the filename and send the file
            char filename[256];
            sscanf(command + 4, "%s", filename);
            send_file(client_socket, filename);
        } else {
            const char *error_message = "Invalid command.\n";
            send(client_socket, error_message, strlen(error_message), 0);
        }
    }

    close(client_socket);
    close(server_fd);
    return 0;
}