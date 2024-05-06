#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

#define SHM_SIZE 1024
#define SHM_KEY_FILE "/Users/rrrreins/sisop/mod3-soal1/shm_key.txt" // Menambahkan definisi nama file untuk key shared memory

void move_to_database(const char *filename, const char *filetype) {
    // Baca key shared memory dari file
    FILE *key_file = fopen(SHM_KEY_FILE, "r");
    if (key_file == NULL) {
        perror("Error opening key file");
        return;
    }
    key_t shm_key;
    fscanf(key_file, "%x", &shm_key);
    fclose(key_file);

    // Access the shared memory segment
    int shm_id = shmget(shm_key, SHM_SIZE, 0666);
    if (shm_id == -1) {
        perror("Error accessing shared memory");
        return;
    }

    // Get information about the shared memory segment
    struct shmid_ds shm_info;
    if (shmctl(shm_id, IPC_STAT, &shm_info) == -1) {
        perror("Error getting shared memory info");
        return;
    }

    // Check if there is any data in shared memory
    if (shm_info.shm_segsz == 0) {
        printf("No data found in shared memory.\n");
        return;
    }

    // Attach the shared memory segment
    char *shm_ptr = (char *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (char *)(-1)) {
        perror("Error attaching shared memory");
        return;
    }

    // Create the database file
    char db_filename[256];
    sprintf(db_filename, "/Users/rrrreins/sisop/mod3-soal1/microservices/database/%s", filename);
    FILE *database_file = fopen(db_filename, "w");
    if (database_file == NULL) {
        perror("Error creating database file");
        shmdt(shm_ptr);
        return;
    }

    // Write the data from shared memory to the database file
    fprintf(database_file, "%s", shm_ptr);

    // Detach shared memory
    shmdt(shm_ptr);

    // Close the database file
    fclose(database_file);

    // Log the file processing
    FILE *log_file = fopen("/Users/rrrreins/sisop/mod3-soal1/microservices/database/db.log", "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }
    time_t now;
    time(&now);
    char timestamp[20];
    strftime(timestamp, 20, "%d/%m/%Y %H:%M:%S", localtime(&now));
    fprintf(log_file, "[%s] [%s] [%s]\n", timestamp, filetype, filename);
    fclose(log_file);
}

int is_valid_file(const char *filename) {
    // Check if the file contains "trashcan" or "parkinglot"
    if (strstr(filename, "trashcan") != NULL || strstr(filename, "parkinglot") != NULL) {
        return 1;
    }
    return 0;
}

int main() {
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir("/Users/rrrreins/sisop/mod3-soal1/new-data")) != NULL) {
        // Iterate over files in the directory
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_REG) { // Regular file
                char filepath[1024];
                sprintf(filepath, "/Users/rrrreins/sisop/mod3-soal1/new-data/%s", ent->d_name);

                if (is_valid_file(ent->d_name)) {
                    // Move valid file to database
                    move_to_database(ent->d_name, strstr(ent->d_name, "trashcan") != NULL ? "Trash Can" : "Parking Lot");
                    printf("File %s moved to database.\n", ent->d_name);
                } else {
                    printf("Skipped invalid file: %s\n", ent->d_name);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Error opening directory");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
