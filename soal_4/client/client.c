#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // Create socket
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

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        printf("You: ");
        fgets(buffer, 1024, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        send(sock, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting the client\n");
            break;
        }

        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, 1024);
        printf("Server:\n%s\n", buffer);
    }

    close(sock);
    return 0;
}
