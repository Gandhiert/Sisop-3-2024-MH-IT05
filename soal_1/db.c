#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 1024

void writeToLog(const char *type, const char *filename) {
    time_t current_time;
    struct tm *local_time;
    char timeString[20];

    current_time = time(NULL);
    local_time = localtime(&current_time);
    strftime(timeString, sizeof(timeString), "%d/%m/%Y %H:%M:%S", local_time);

    FILE *logFile = fopen("/Users/rrrreins/sisop/mod3-so1/microservices/database/db.log", "a");
    if (logFile == NULL) {
        perror("fopen");
        exit(1);
    }
    fprintf(logFile, "[%s] [%s] [%s]\n", timeString, type, filename);
    fclose(logFile);
}

int main() {
    int baseKey = 12345678; // Base key for shared memory segments
    int shmid;
    char *shared_memory;

    DIR *dir = opendir("/Users/rrrreins/sisop/mod3-so1/new-data");
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        if ((strstr(entry->d_name, "trashcan.csv") || strstr(entry->d_name, "parkinglot.csv"))) {
            int key = baseKey; // Start with the base key
            int shmid;
            char *filename = entry->d_name;

            // Find the corresponding shared memory segment
            while ((shmid = shmget(key, MAX_FILENAME_LENGTH, 0666)) != -1) {
                shared_memory = shmat(shmid, NULL, 0);
                if (shared_memory == (char *) -1) {
                    perror("shmat");
                    exit(1);
                }

                if (strcmp(shared_memory, filename) == 0) {
                    // Copy the file to the database directory
                    char source[MAX_FILENAME_LENGTH];
                    char destination[MAX_FILENAME_LENGTH];
                    snprintf(source, sizeof(source), "/Users/rrrreins/sisop/mod3-so1/new-data/%s", filename);
                    snprintf(destination, sizeof(destination), "/Users/rrrreins/sisop/mod3-so1/microservices/database/%s", filename);

                    FILE *sourceFile = fopen(source, "r");
                    if (sourceFile == NULL) {
                        perror("fopen");
                        exit(1);
                    }

                    FILE *destinationFile = fopen(destination, "w");
                    if (destinationFile == NULL) {
                        perror("fopen");
                        exit(1);
                    }

                    char buffer[BUFSIZ];
                    size_t bytes_read;
                    while ((bytes_read = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
                        fwrite(buffer, 1, bytes_read, destinationFile);
                    }

                    fclose(sourceFile);
                    fclose(destinationFile);

                    // Write to log
                    const char *type = strstr(filename, "trashcan.csv") ? "Trash Can" : "Parking Lot";
                    writeToLog(type, filename);

                    // Detach shared memory
                    shmdt(shared_memory);
                    
                    break; // Move to the next file
                }

                // Detach shared memory
                shmdt(shared_memory);
                key++; // Move to the next shared memory segment
            }
        }
    }

    closedir(dir);

    return 0;
}
