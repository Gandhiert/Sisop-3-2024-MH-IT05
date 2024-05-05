#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "actions.c"

#define PORT 8080
#define MAX_BUFFER 1024

void logMessage(char* source, char* command, char* info) {
    time_t now;
    struct tm* timeinfo;
    char timestamp[20];

    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%d/%m/%y %H:%M:%S", timeinfo);

    FILE* logFile = fopen("race.log", "a");
    if (logFile == NULL) {
        printf("Error opening log file.\n");
        return;
    }

    fprintf(logFile, "[%s] [%s]: [%s] [%s]\n", source, timestamp, command, info);
    fclose(logFile);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[MAX_BUFFER] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if (read(new_socket, buffer, MAX_BUFFER) < 0) {
            perror("read");
            close(new_socket);
            continue;
        }

        // Parse the received command and additional info
        char command[20], info[MAX_BUFFER];
        sscanf(buffer, "%s %[^\n]", command, info);

        logMessage("Driver", command, info);

        char response[MAX_BUFFER];
        if (strcmp(command, "Gap") == 0) {
            float distance = atof(info);
            strcpy(response, gap(distance));
        } else if (strcmp(command, "Fuel") == 0) {
            float percentage = atof(info);
            strcpy(response, fuel(percentage));
        } else if (strcmp(command, "Tire") == 0) {
            int wear = atoi(info);
            strcpy(response, tire(wear));
        } else if (strcmp(command, "Tire Change") == 0) {
            strcpy(response, tireChange(info));
        } else {
            strcpy(response, "Invalid command");
        }

        logMessage("Paddock", command, response);

        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }

    return 0;
}
