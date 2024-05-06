#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dirent.h>
#include <sys/stat.h>

#define SHM_SIZE 1024
#define SHM_KEY_FILE "shm_key.txt" // Nama file untuk menyimpan key shared memory

int is_valid_file(const char *filename) {
    // Check if the filename contains "trashcan" or "parkinglot"
    if (strstr(filename, "trashcan") != NULL || strstr(filename, "parkinglot") != NULL) {
        return 1;
    }
    return 0;
}

void move_to_shared_memory(const char *filename) {
    // Open the file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    // Get the size of the file
    fseek(file, 0L, SEEK_END);
    long int file_size = ftell(file);
    rewind(file);

    // Generate a new unique key
    key_t key = ftok(filename, 'R');
    if (key == -1) {
        perror("Error generating key");
        fclose(file);
        return;
    }

    // Simpan key ke dalam file
    FILE *key_file = fopen(SHM_KEY_FILE, "w");
    if (key_file == NULL) {
        perror("Error opening key file");
        fclose(file);
        return;
    }
    fprintf(key_file, "%x", key);
    fclose(key_file);

    // Create or access the shared memory segment
    int shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Error creating/accessing shared memory");
        fclose(file);
        return;
    }

    // Attach the shared memory segment
    char *shm_ptr = (char *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (char *)(-1)) {
        perror("Error attaching shared memory");
        fclose(file);
        return;
    }

    // Write the file contents to shared memory
    fread(shm_ptr, sizeof(char), file_size, file);

    // Detach the shared memory segment
    shmdt(shm_ptr);

    // Close the file
    fclose(file);
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
                    // Move valid file to shared memory
                    move_to_shared_memory(filepath);
                    printf("File %s moved to shared memory.\n", ent->d_name);
                } else {
                    // Invalid file, delete it
                    remove(filepath);
                    printf("Invalid file %s deleted.\n", ent->d_name);
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
