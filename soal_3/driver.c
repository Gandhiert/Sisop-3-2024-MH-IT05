#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define MAX_BUFFER 1024

int main(int argc, char const* argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    if (argc < 3) {
        printf("Usage: %s -c <command> -i <info>\n", argv[0]);
        return -1;
    }

    char command[20], info[MAX_BUFFER];
    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-c") == 0) {
            strcpy(command, argv[i + 1]);
        } else if (strcmp(argv[i], "-i") == 0) {
            strcpy(info, argv[i + 1]);
        }
    }

    char message[MAX_BUFFER];
    snprintf(message, sizeof(message), "%s %s", command, info);

    send(sock, message, strlen(message), 0);
    printf("Command sent: %s\n", message);

    if (read(sock, buffer, MAX_BUFFER) < 0) {
        printf("Error reading response\n");
        return -1;
    }

    printf("Response from paddock: %s\n", buffer);

    close(sock);
    return 0;
}
