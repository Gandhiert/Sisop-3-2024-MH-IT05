#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_PLACES 100
#define SHM_KEY_FILE "/Users/rrrreins/sisop/mod3-soal1/shm_key.txt"

// Struktur untuk data tempat sampah atau area parkir
typedef struct {
    char name[50];
    float rating;
} PlaceInfo;

key_t get_shm_key() {
    FILE *key_file = fopen(SHM_KEY_FILE, "r");
    if (key_file == NULL) {
        perror("Error opening key file");
        exit(EXIT_FAILURE);
    }

    key_t key;
    fscanf(key_file, "%x", &key);
    fclose(key_file);
    return key;
}

void print_highest_rated(PlaceInfo places[], int num_places, const char *filename) {
    if (num_places == 0) {
        printf("No places found in %s\n", filename);
        return;
    }

    // Cari tempat dengan rating tertinggi
    PlaceInfo highest_rated = places[0];
    for (int i = 1; i < num_places; i++) {
        if (places[i].rating > highest_rated.rating) {
            highest_rated = places[i];
        }
    }

    // Tampilkan hasil
    printf("Rating tertinggi di %s:\n", filename);
    printf("--------------------------------------------------\n");
    printf("Nama: %s\n", highest_rated.name);
    printf("Rating: %.1f\n", highest_rated.rating);
    printf("--------------------------------------------------\n");
}

int main() {
    // Kunci yang diperoleh dari file
    key_t key = get_shm_key();
    int shmid;
    PlaceInfo *shared_places;

    // Mengakses shared memory
    if ((shmid = shmget(key, MAX_PLACES * sizeof(PlaceInfo), 0666)) < 0) {
        perror("shmget");
        return 1;
    }

    // Me-attach shared memory
    if ((shared_places = (PlaceInfo *) shmat(shmid, NULL, 0)) == (PlaceInfo *) -1) {
        perror("shmat");
        return 1;
    }

    // Hitung jumlah tempat yang ada di shared memory
    int num_places = shmid / sizeof(PlaceInfo);

    // Tampilkan hasil untuk semua file
    print_highest_rated(shared_places, num_places, "All Files");

    // Me-detach shared memory
    if (shmdt(shared_places) == -1) {
        perror("shmdt");
        return 1;
    }

    return 0;
}
