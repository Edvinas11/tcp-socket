#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 20000
#define BUFFER_SIZE 1024

void send_file(int socket, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File open failed");
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(socket, buffer, bytes_read, 0);
    }

    printf("File sent successfully: %s\n", filename);
    fclose(file);
}

int main() {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    char filename[256];
    printf("Enter filename to send: ");
    scanf("%s", filename);
    
    send_file(sock, filename);

    close(sock);
    return 0;
}