#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define SERVER_IP "127.0.0.1"
#define PORT 20000
#define BUFFER_SIZE 1024
#define CLIENT_DIR "client_dir"

// Function to send a file to the server
void send_file(int sock, const char *filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", CLIENT_DIR, filename);

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("File open failed");
        return;
    }

    printf("Sending file: %s\n", filename);

    char buffer[BUFFER_SIZE];
    int bytes_read;
    int total_bytes_sent = 0;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        printf("Sending %d bytes\n", bytes_read);  // Debugging line

        if (send(sock, buffer, bytes_read, 0) == -1) {
            perror("Send failed");
            fclose(file);
            return;
        }

        total_bytes_sent += bytes_read;
    }

    printf("Total bytes sent: %d\n", total_bytes_sent);
    printf("File sent successfully: %s\n", filename);

    fclose(file);
    // Close the socket here to notify server that the transfer is complete
    close(sock);
}

// Function to receive a file from the server
void receive_file(int sock, const char *filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", CLIENT_DIR, filename);

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        perror("File open failed");
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, file);
    }

    printf("File received successfully: %s\n", filepath);
    fclose(file);
}

int main() {
    int sock;
    struct sockaddr_in server_addr;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Client loop
    char command[256];
    printf("Enter command (get <filename> or put <filename>): ");
    fgets(command, sizeof(command), stdin);
    
    // Remove newline character from the input
    command[strcspn(command, "\n")] = 0;

    if (strlen(command) == 0) {
        printf("No command entered.\n");
        close(sock);
        return 0;
    }

    // Send the command to the server
    send(sock, command, strlen(command), 0);

    if (strncmp(command, "put ", 4) == 0) {
        // Extract the filename to upload
        char filename[256];
        sscanf(command + 4, "%s", filename);
        send_file(sock, filename); // Send the file
    } else if (strncmp(command, "get ", 4) == 0) {
        // Extract the filename to download
        char filename[256];
        sscanf(command + 4, "%s", filename);
        receive_file(sock, filename); // Receive the file
    } else {
        printf("Invalid command.\n");
    }

    close(sock);
    return 0;
}