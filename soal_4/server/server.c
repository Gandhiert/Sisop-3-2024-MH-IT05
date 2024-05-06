#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <curl/curl.h>

#define PORT 8080

void writeLog(const char *type, const char *message) {
    FILE *fp = fopen("../change.log", "a");
    if (fp == NULL) {
        printf("Error opening log file\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[20];
    strftime(date, sizeof(date), "%d/%m/%y", t);

    fprintf(fp, "[%s] [%s] %s\n", date, type, message);
    fclose(fp);
}

void readCSV(const char *filename, char *buffer) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        strcpy(buffer, "File not found");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), fp)) {
        strcat(buffer, line);
    }

    fclose(fp);
}

void addAnime(const char *filename, const char *anime) {
    FILE *fp = fopen(filename, "a");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }

    fprintf(fp, "%s\n", anime);
    fclose(fp);

    char message[100];
    snprintf(message, sizeof(message), "%s ditambahkan.", anime);
    writeLog("ADD", message);
}

void editAnime(const char *filename, const char *oldAnime, const char *newAnime) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }

    FILE *tempFp = fopen("temp.csv", "w");
    if (tempFp == NULL) {
        printf("Error opening temporary file\n");
        fclose(fp);
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, oldAnime) != NULL) {
            fprintf(tempFp, "%s\n", newAnime);
        } else {
            fprintf(tempFp, "%s", line);
        }
    }

    fclose(fp);
    fclose(tempFp);

    remove(filename);
    rename("temp.csv", filename);

    char message[200];
    snprintf(message, sizeof(message), "%s diubah menjadi %s.", oldAnime, newAnime);
    writeLog("EDIT", message);
}

void deleteAnime(const char *filename, const char *anime) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        return;
    }

    FILE *tempFp = fopen("temp.csv", "w");
    if (tempFp == NULL) {
        printf("Error opening temporary file\n");
        fclose(fp);
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, anime) == NULL) {
            fprintf(tempFp, "%s", line);
        }
    }

    fclose(fp);
    fclose(tempFp);

    remove(filename);
    rename("temp.csv", filename);

    char message[100];
    snprintf(message, sizeof(message), "%s berhasil dihapus.", anime);
    writeLog("DEL", message);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void downloadCSV(const char *url, const char *filename) {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        fp = fopen(filename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        fclose(fp);
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Create socket file descriptor
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

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Mengunduh file CSV dari link Google Drive ke direktori utama soal_4
    const char *url = "https://drive.google.com/uc?id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50&export=download";
    const char *filename = "../myanimelist.csv";
    downloadCSV(url, filename);

    while (1) {
        printf("Waiting for incoming connections...\n");

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int valread = read(new_socket, buffer, 1024);
            if (valread <= 0) {
                break;
            }

            printf("Received: %s\n", buffer);

            if (strcmp(buffer, "genre Slice of Life") == 0) {
                memset(buffer, 0, sizeof(buffer));
                readCSV("../myanimelist.csv", buffer);
                send(new_socket, buffer, strlen(buffer), 0);
            } else if (strncmp(buffer, "genre", 5) == 0) {
                char genre[50];
                sscanf(buffer, "genre %[^\n]", genre);

                memset(buffer, 0, sizeof(buffer));
                readCSV("../myanimelist.csv", buffer);
                char *token = strtok(buffer, "\n");
                while (token != NULL) {
                    if (strstr(token, genre) != NULL) {
                        strcat(buffer, token);
                        strcat(buffer, "\n");
                    }
                    token = strtok(NULL, "\n");
                }
                send(new_socket, buffer, strlen(buffer), 0);
            } else if (strcmp(buffer, "hari Rabu") == 0) {
                memset(buffer, 0, sizeof(buffer));
                readCSV("../myanimelist.csv", buffer);
                send(new_socket, buffer, strlen(buffer), 0);
            } else if (strncmp(buffer, "hari", 4) == 0) {
                char hari[20];
                sscanf(buffer, "hari %[^\n]", hari);

                memset(buffer, 0, sizeof(buffer));
                readCSV("../myanimelist.csv", buffer);
                char *token = strtok(buffer, "\n");
                while (token != NULL) {
                    if (strncmp(token, hari, strlen(hari)) == 0) {
                        strcat(buffer, token);
                        strcat(buffer, "\n");
                    }
                    token = strtok(NULL, "\n");
                }
                send(new_socket, buffer, strlen(buffer), 0);
            } else if (strcmp(buffer, "status ongoing") == 0) {
                memset(buffer, 0, sizeof(buffer));
                readCSV("../myanimelist.csv", buffer);
                char *token = strtok(buffer, "\n");
                while (token != NULL) {
                    if (strstr(token, "ongoing") != NULL) {
                        strcat(buffer, token);
                        strcat(buffer, "\n");
                    }
                    token = strtok(NULL, "\n");
                }
                send(new_socket, buffer, strlen(buffer), 0);
            } else if (strncmp(buffer, "add", 3) == 0) {
                char anime[100];
                sscanf(buffer, "add %[^\n]", anime);
                addAnime("../myanimelist.csv", anime);
                strcpy(buffer, "anime berhasil ditambahkan.");
                send(new_socket, buffer, strlen(buffer), 0);
            } else if (strncmp(buffer, "edit", 4) == 0) {
                char oldAnime[100], newAnime[100];
                sscanf(buffer, "edit %[^,],%[^\n]", oldAnime, newAnime);
                editAnime("../myanimelist.csv", oldAnime, newAnime);
                strcpy(buffer, "anime berhasil diedit");
                send(new_socket, buffer, strlen(buffer), 0);
            } else if (strncmp(buffer, "delete", 6) == 0) {
                char anime[100];
                sscanf(buffer, "delete %[^\n]", anime);
                deleteAnime("../myanimelist.csv", anime);
                strcpy(buffer, "anime berhasil dihapus");
                send(new_socket, buffer, strlen(buffer), 0);
            } else {
                strcpy(buffer, "Invalid Command");
                send(new_socket, buffer, strlen(buffer), 0);
            }
        }

        close(new_socket);
    }

    return 0;
}
