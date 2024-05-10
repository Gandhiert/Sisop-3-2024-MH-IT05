#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>

int main() {
    // Opening directory
    DIR *dir = opendir("new-data");
    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }

    // Iterate through files in the directory
    struct dirent *entry;
    int shm_key = 12345678; // Shared memory key
    while ((entry = readdir(dir)) != NULL) {
        // Validating file extension
        if (strstr(entry->d_name, "trashcan.csv") || strstr(entry->d_name, "parkinglot.csv")) {
            // Creating or getting shared memory segment
            int shmid = shmget(shm_key, 1024, IPC_CREAT | 0666);
            if (shmid == -1) {
                fprintf(stderr, "Failed to create or get shared memory segment for file: %s\n", entry->d_name);
                perror("shmget");
                exit(1);
            }

            // Attaching shared memory
            char *shared_memory = shmat(shmid, NULL, 0);
            if (shared_memory == (char *) -1) {
                fprintf(stderr, "Failed to attach shared memory for file: %s\n", entry->d_name);
                perror("shmat");
                exit(1);
            }

            // Writing filename to shared memory
            strcpy(shared_memory, entry->d_name);

            // Detaching shared memory
            shmdt(shared_memory);

            // Incrementing the key for the next shared memory segment
            shm_key++;

            printf("File successfully moved to shared memory: %s\n", entry->d_name);
        } else {
            // Deleting invalid file
            char filePath[1024];
            snprintf(filePath, sizeof(filePath), "new-data/%s", entry->d_name);
            remove(filePath);
            printf("Invalid file deleted: %s\n", entry->d_name);
        }
    }

    // Closing directory
    closedir(dir);

    return 0;
}
