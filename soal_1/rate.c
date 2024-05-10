#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_FILENAME_LENGTH 1024

int main() {
    int baseKey = 12345678; // Base key for shared memory segments
    int shmid;
    char *shared_memory;

    // Iterate through each shared memory segment
    for (int i = 0; ; i++) {
        int key = baseKey + i; // Calculate key for this segment
        shmid = shmget(key, MAX_FILENAME_LENGTH, 0666);
        if (shmid == -1) {
            break;
        }
        shared_memory = shmat(shmid, NULL, 0);
        if (shared_memory == (char *) -1) {
            perror("shmat");
            exit(1);
        }

        // Read filename from shared memory
        char *filename = shared_memory;
        const char *type;
        if (strstr(filename, "trashcan.csv")) {
            type = "Trashcan";
        } else {
            type = "Parking lot";
        }

        // Construct full file path
        char fullpath[MAX_FILENAME_LENGTH];
        snprintf(fullpath, sizeof(fullpath), "/Users/rrrreins/sisop/mod3-so1/new-data/%s", filename);

        // Open file
        FILE *file = fopen(fullpath, "r");
        if (!file) {
            fprintf(stderr, "Error: Could not open file: %s\n", fullpath);
            perror("fopen");
            exit(1);
        }

        char line[MAX_FILENAME_LENGTH];
        float maxRating = 0.0;
        char bestName[MAX_FILENAME_LENGTH] = {0};

        // Read and process the file
        while (fgets(line, sizeof(line), file)) {
            char *name = strtok(line, ",");
            char *rating_str = strtok(NULL, ",");
            float rating = atof(rating_str);

            if (rating > maxRating) {
                maxRating = rating;
                strcpy(bestName, name);
            }
        }

        fclose(file);

        // Print the results
        printf("Type: %s\n", type);
        printf("Filename: %s\n", filename);
        printf("------------\n");
        if (maxRating > 0.0) {
            printf("Name: %s\n", bestName);
            printf("Rating: %.1f\n\n", maxRating);
        } else {
            printf("No data available.\n\n");
        }

        // Detach shared memory
        shmdt(shared_memory);
    }

    return 0;
}
