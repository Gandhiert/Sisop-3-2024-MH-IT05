<div align=center>

# Laporan Pengerjaan - Praktikum Modul 3 Sistem Operasi

</div>


## Kelompok IT05 - MH
Fikri Aulia As Sa'adi - 5027231026

Aisha Ayya Ratiandari - 5027231056

Gandhi Ert Julio - 5027231081

## _Soal 1_

### Dikerjakan oleh Aisha Ayya R (5027231056)

Dalam soal pertama ini, diminta untuk membuat tiga file c yang secara otomatis bisa mengetahui Tempat Sampah dan Parkiran dengan rating terbaik di angkasa. 

### auth.c

```c
#define MAX_FILENAME_LENGTH 512
#define MAX_FILE_CONTENT_LENGTH 1024
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 4321
#define SHARED_MEMORY_SIZE (sizeof(FileInfo) * MAX_FILES)
```

define untuk mengatur ukuran maksimum nama filenya, isi konten file, jumlah maksimum file, kunci dari shared memory, dan ukuran total shared memory-nya.

```c
typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
} FileInfo;

```

menggunakan struct untuk menyimpan informasi tentang setiap file yang akan disimpan di shared memory, termasuk nama file dan kontennya.

```c
int cek_file(const char *filename) {
    // Cek apakah nama file berakhiran dengan "parkinglot.csv" atau "trashcan.csv"
    if (strstr(filename, "parkinglot.csv") || strstr(filename, "trashcan.csv")) {
        return 1;
    } else {
        return 0; // File selain csv tidak valid
    }
}
```

fungsi “cek_file” untuk memeriksa apakah nama file sesuai dengan kriteria tertentu, sesuai dengan apa yang diminta soal, yaitu:

Dalam **auth.c** pastikan file yang masuk ke folder **new-entry** adalah file *csv* dan berakhiran  **trashcan** dan **parkinglot**. Jika bukan, program akan secara langsung akan delete file tersebut.

```c
void file_lolos() {
    DIR *dir;
    struct dirent *entry;

    // Membuka direktori folder yang diinginkan, new-data
    dir = opendir("new-data");
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    // Membuat shared memory
    int shmid = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    // Menghubungkan shared memory ke ruang alamat proses
    FileInfo *shmaddr = (FileInfo *)shmat(shmid, NULL, 0);
    if (shmaddr == (FileInfo *)-1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    int file_count = 0;

    // Looping untuk memindah file menuju shared memory
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (entry->d_type == DT_REG) { // Memeriksa jika entri adalah file regular
            char filename[MAX_FILENAME_LENGTH];
            snprintf(filename, MAX_FILENAME_LENGTH, "new-data/%s", entry->d_name);

            // Menyalin nama file dan isi file jika sesuai kriteria
            if (cek_file(entry->d_name)) {
                strcpy(shmaddr[file_count].filename, entry->d_name);
                FILE *fp = fopen(filename, "r");
                if (fp == NULL) {
                    perror("Error opening file");
                    exit(EXIT_FAILURE);
                }
                fread(shmaddr[file_count].content, 1, MAX_FILE_CONTENT_LENGTH, fp);
                fclose(fp);

                printf("File berhasil disimpan di shared memory:\t[%s]\n", shmaddr[file_count].filename);
                file_count++;
            } else {
                // File yang tidak sesuai akan dihapus
                printf("File tidak valid, dihapus:\t[%s]\n", entry->d_name);
                remove(filename);
            }
        }
    }

    // Melepaskan shared memory
    shmdt((void *)shmaddr);
    closedir(dir);
}
```

dalam fungsi “filelolos” adalah untuk memproses file yang sudah memenuhi kriteria akan diproses, membuka direktori yang diinginkan, membuat shared memory, menghubungkan shared memory ke ruang alamat proses, memindahkan file-file yang lolos autentikasi ke dalam shared memory, serta melepaskan shared memory setelah selesai.
nanti fungsi ini akan dijalankan dalam fungsi utama

### rate.c

```c
#define MAX_FILENAME_LENGTH 512
#define MAX_FILE_CONTENT_LENGTH 1024
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 4321
#define SHARED_MEMORY_SIZE (sizeof(FileInfo) * MAX_FILES)

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
} FileInfo;
```

menggunakan define dan struct yang sama dengan program auth.c

```c
void nama_rating(const char *content, char *name, float *rating) {
    sscanf(content, "%[^,], %f", name, rating);
}
```

untuk memisahkan nama dan rating dari konten file CSV.

```
void print_rating(const char *filename, const char *content) {
    char temp_content[MAX_FILE_CONTENT_LENGTH];
    strcpy(temp_content, content);
    float highest_rating = 0.0;
    char highest_rated_name[MAX_FILENAME_LENGTH] = "";
    char *token = strtok(temp_content, "\n");
    token = strtok(NULL, "\n");

    while (token != NULL) {
        char name[MAX_FILENAME_LENGTH];
        float rating;
        nama_rating(token, name, &rating);

        if (strcmp(filename, name) == 0) {
            highest_rating = rating;
            strcpy(highest_rated_name, name);
            break;
        }

        token = strtok(NULL, "\n");
    }

    printf("Filename: %s\n", filename);
    printf("Rating: %.1f\n", highest_rating);
}
```

lalu fungsi ini adalah untuk mencetak rating tertinggi dari isi file CSV berdasarkan nama file yang diminta. Saat dipanggil, fungsi ini mengakses konten file yang disimpan dalam shared memory, memproses konten tersebut untuk mencari rating tertinggi, dan mencetak informasi tentang file yang diminta beserta rating tertinggi yang ditemukan. Dalam prosesnya, konten file dibagi menjadi baris-baris terpisah yang dianalisis secara berurutan. Jika nama file yang diminta cocok dengan nama dalam baris, rating dari baris tersebut akan diperiksa. Jika ratingnya lebih tinggi dari rating tertinggi yang sebelumnya disimpan, rating tertinggi dan nama dengan rating tertinggi akan diperbarui. Setelah semua baris diperiksa, fungsi mencetak informasi tentang file yang diminta dan rating tertinggi yang ditemukan. Jika tidak ada rating yang ditemukan untuk file yang diminta, fungsi akan mencetak nilai default untuk rating. Dengan demikian, fungsi ini  memberikan informasi tentang rating tertinggi dari file yang diminta dengan menggunakan shared memory sebagai media penyimpanannya.

```c
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *requested_filename = argv[1];

    // Mendapatkan akses ke shared memory yang sama
    int shmid = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, 0666);
    if (shmid < 0) {
        perror("Error accessing shared memory");
        exit(EXIT_FAILURE);
    }

    // Menghubungkan shared memory ke ruang alamat proses
    FileInfo *shmaddr = (FileInfo *)shmat(shmid, NULL, 0);
    if (shmaddr == (FileInfo *)-1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    // Membaca dan mencetak rating tertinggi dari isi file di shared memory
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strlen(shmaddr[i].filename) > 0) {
            if (strcmp(shmaddr[i].filename, requested_filename) == 0) {
                print_rating(shmaddr[i].filename, shmaddr[i].content);
                break;
            }
        }
    }

    // Melepaskan shared memory
    shmdt((void *)shmaddr);

    return 0;
}
```

dalam fungsi utama ini program dieksekusi secara keseluruhan.

- Pertama, fungsi tersebut memeriksa apakah argumen baris perintah yang diberikan oleh pengguna sesuai dengan yang diharapkan, yaitu nama file yang diminta untuk dicari rating tertingginya. Jika jumlah argumen tidak sesuai, program akan mencetak pesan tentang cara penggunaan yang benar dan keluar dengan status kegagalan.
- Setelah itu, nama file yang diminta disimpan dalam variabel “requested_filename”. Selanjutnya, program mengakses shared memory yang sama dengan menggunakan kunci yang telah ditentukan sebelumnya. Jika akses ke shared memory gagal, program akan mencetak pesan kesalahan dan keluar dengan status kegagalan.
- Kemudian, shared memory akan dihubungkan ke ruang alamat proses. Dalam loop, program mencari file dengan nama yang sesuai dalam shared memory. Ketika file yang diminta ditemukan, program memanggil fungsi “print_rating” untuk mencetak rating tertinggi dari isi file tersebut. Setelah  selesai, program melepaskan shared memory dan kemudian berakhir dengan status kesuksesan.
- Sehingga, fungsi ini mengatur alur kerja program, mulai dari memeriksa argumen masukan, mengakses shared memory, mencari file yang diminta, hingga mencetak rating tertinggi dan menutup akses ke shared memory.

### db.c

```c
#define MAX_FILENAME_LENGTH 512
#define MAX_FILE_CONTENT_LENGTH 1024
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 4321
#define SHARED_MEMORY_SIZE (sizeof(FileInfo) * MAX_FILES)

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
} FileInfo;
```

menggunakan define dan struct yang sama dengan auth.c dan rate.c

```c
void tipe_file(const char *filename, char *type) {
    if (strstr(filename, "trashcan") || strstr(filename, "TrashCan")) {
        strcpy(type, "Trash Can");
    } else if (strstr(filename, "parkinglot") || strstr(filename, "ParkingLot")) {
        strcpy(type, "Parking Lot");
    }
}
```

Fungsi “tipe file” digunakan untuk menentukan tipe file berdasarkan nama file yang diberikan. Jika nama file mengandung substring "trashcan" atau "TrashCan", maka tipe file akan diset sebagai "Trash Can". Jika tidak, namun mengandung substring "parkinglot" atau "ParkingLot", maka tipe file akan diset sebagai "Parking Lot".

```c
void catat_log(const char *filename, const char *type) {
    time_t current_time;
    struct tm *local_time;
    char time_string[80];
    
    current_time = time(NULL);
    local_time = localtime(&current_time);
    strftime(time_string, sizeof(time_string), "[%d/%m/%Y %H:%M:%S]", local_time);
    
    // Mencari dan membuka file db.log
    FILE *log_file = fopen("database/db.log", "a");
    if (log_file == NULL) {
        log_file = fopen("database/db.log", "w");
        if (log_file == NULL) {
            perror("Error creating log file");
            exit(EXIT_FAILURE);
        }
        fprintf(log_file, "Log File Created\n");
    }
    fprintf(log_file, "%s\t[%s]\t[%s]\n", time_string, type, filename);
    fclose(log_file);
}
```

fungsi **`catat_log()`** bertanggung jawab untuk mencatat informasi tentang file ke dalam file log "db.log". Fungsi ini pertama-tama mendapatkan waktu lokal saat ini dan mengonversinya menjadi format string yang sesuai. Selanjutnya, fungsi membuka file log "db.log" dalam mode "append". Jika file log tidak dapat dibuka, maka fungsi akan mencoba membuat file log baru. Setelah itu, informasi tentang file, termasuk waktu, tipe file, dan nama file, dicatat dalam file log dengan format yang telah ditentukan. Akhirnya, file log ditutup setelah pencatatan selesai. Dengan demikian, kedua fungsi ini memiliki peran penting dalam pengelolaan file dan pencatatan informasi terkait dalam program.

```c
int main() {
    // Mendapatkan akses ke shared memory yang sama
    int shmid = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, 0666);
    if (shmid < 0) {
        perror("Error accessed shared memory");
        exit(EXIT_FAILURE);
    }

    // Menghubungkan shared memory ke ruang alamat proses
    FileInfo *shmaddr = (FileInfo *)shmat(shmid, NULL, 0);
    if (shmaddr == (FileInfo *)-1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    // Mendapatkan jalur direktori saat ini
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error getting current directory");
        exit(EXIT_FAILURE);
    }

    // Menambahkan "/database" ke jalur direktori saat ini
    strcat(cwd, "/database");

    // Membuat folder database jika belum ada
    struct stat st = {0};
    if (stat(cwd, &st) == -1) {
        mkdir(cwd, 0700);
    }

    // Menyalin file dari shared memory ke folder database
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strlen(shmaddr[i].filename) > 0) {
            char dest_path[MAX_FILENAME_LENGTH + 1024];
            snprintf(dest_path, sizeof(dest_path), "%s/%s", cwd, shmaddr[i].filename);
            FILE *fp = fopen(dest_path, "w");
            if (fp == NULL) {
                perror("Error creating file");
                continue;
            }
            fwrite(shmaddr[i].content, 1, strlen(shmaddr[i].content), fp);
            fclose(fp);
            printf("Berhasil memindah file ke database : %s\n", shmaddr[i].filename);

            // Menentukan jenis berdasarkan nama file
            char type[MAX_FILENAME_LENGTH];
            tipe_file(shmaddr[i].filename, type);

            // Mencatat log
            catat_log(shmaddr[i].filename, type);

            // Menghapus file asli dari folder new-data setelah menyalinnya
            char source_path[MAX_FILENAME_LENGTH + 1000];
            snprintf(source_path, sizeof(source_path), "../new-data/%s", shmaddr[i].filename);
            if (remove(source_path) != 0) {
                perror("Error deleting original file");
            }
        }
    }

    // Melepaskan shared memory
    shmdt((void *)shmaddr);
    return 0;
}
```

Semua fungsi tersebut akan dijalankan di fungsi utama ini.

- Pertama, program mengakses shared memory yang sama dengan menggunakan kunci yang telah ditentukan sebelumnya. Kemudian, program menghubungkan shared memory tersebut ke ruang alamat proses.
- Lalu, program mendapatkan jalur direktori saat ini dan membuat folder "database" jika belum ada. Setelah itu, program menyalin file dari shared memory ke folder "database", kemudian menentukan jenis file berdasarkan nama dan mencatat informasi file ke dalam file log.
- Sehingga akhirnya, program menghapus file asli dari folder "new-data" setelah menyalinnya. Setelah semua operasi selesai, program melepaskan shared memory dan mengakhiri eksekusi.

## _Soal 2_

### Dikerjakan oleh Fikri Aulia A (5027231026)

Di soal no.2, diminta untuk membuat sebuah kalkulator yang dapat mengerjakan operasi sesuai request dari user, dengan ketentuan sebagai berikut:

- Sesuai request dari adiknya Max ingin nama programnya dudududu.c. Sebelum program parent process dan child process, ada input dari user berupa 2 string. Contoh input: tiga tujuh. 


- Pada parent process, program akan mengubah input menjadi angka dan melakukan perkalian dari angka yang telah diubah. Contoh: tiga tujuh menjadi 21. 
Pada child process, program akan mengubah hasil angka yang telah diperoleh dari parent process menjadi kalimat. Contoh: `21` menjadi “dua puluh satu”.


- Max ingin membuat program kalkulator dapat melakukan penjumlahan, pengurangan, dan pembagian, maka pada program buatlah argumen untuk menjalankan program : 
perkalian	: ./kalkulator -kali
penjumlahan	: ./kalkulator -tambah
pengurangan	: ./kalkulator -kurang
pembagian	: ./kalkulator -bagi
Beberapa hari kemudian karena Max terpaksa keluar dari Australian Grand Prix 2024 membuat Max tidak bersemangat untuk melanjutkan programnya sehingga kalkulator yang dibuatnya cuma menampilkan hasil positif jika bernilai negatif maka program akan print “ERROR” serta cuma menampilkan bilangan bulat jika ada bilangan desimal maka dibulatkan ke bawah.


- Setelah diberi semangat, Max pun melanjutkan programnya dia ingin (pada child process) kalimat akan di print dengan contoh format : 
perkalian	: “hasil perkalian tiga dan tujuh adalah dua puluh satu.”
penjumlahan	: “hasil penjumlahan tiga dan tujuh adalah sepuluh.”
pengurangan	: “hasil pengurangan tujuh dan tiga adalah empat.”
pembagian	: “hasil pembagian tujuh dan tiga adalah dua.”


- Max ingin hasil dari setiap perhitungan dicatat dalam sebuah log yang diberi nama histori.log. Pada parent process, lakukan pembuatan file log berdasarkan data yang dikirim dari child process. 
Format: [date] [type] [message]
Type: KALI, TAMBAH, KURANG, BAGI

Ex:
[10/03/24 00:29:47] [KALI] tujuh kali enam sama dengan empat puluh dua.
[10/03/24 00:30:00] [TAMBAH] sembilan tambah sepuluh sama dengan sembilan belas.
[10/03/24 00:30:12] [KURANG] ERROR pada pengurangan.

### Penyelesaian
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>

#define MAX_LENGTH 20

void convertToNumber(char *str, int *num) {
    if (strcmp(str, "nol") == 0) {
        *num = 0;
    } else if (strcmp(str, "satu") == 0) {
        *num = 1;
    } else if (strcmp(str, "dua") == 0) {
        *num = 2;
    } else if (strcmp(str, "tiga") == 0) {
        *num = 3;
    } else if (strcmp(str, "empat") == 0) {
        *num = 4;
    } else if (strcmp(str, "lima") == 0) {
        *num = 5;
    } else if (strcmp(str, "enam") == 0) {
        *num = 6;
    } else if (strcmp(str, "tujuh") == 0) {
        *num = 7;
    } else if (strcmp(str, "delapan") == 0) {
        *num = 8;
    } else if (strcmp(str, "sembilan") == 0) {
        *num = 9;
    }
}

void convertToWords(int num, char *str) {
    char *ones[] = {"nol", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    char *tens[] = {"", "sepuluh", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};

    if (num >= 0 && num <= 9) {
        strcpy(str, ones[num]);
    } else if (num >= 10 && num <= 19) {
        strcpy(str, teens[num - 10]);
    } else if (num >= 20 && num <= 99) {
        strcpy(str, tens[num / 10]);
        if (num % 10 != 0) {
            strcat(str, " ");
            strcat(str, ones[num % 10]);
        }
    }
}

void writeToLog(char *type, char *input1, char *input2, char *operation, char *result) {
    time_t now;
    struct tm *local;
    char timestamp[20];
    FILE *logFile;

    time(&now);
    local = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%y/%m/%d %H:%M:%S", local);

    logFile = fopen("histori.log", "a");
    if (logFile == NULL) {
        perror("Failed to open histori.log");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; type[i] != '\0'; i++) {
        type[i] = toupper(type[i]);
    }

    if (strcmp(result, "ERROR") == 0) {
        fprintf(logFile, "\n [%s] [%s] %s pada pengurangan", timestamp, type, result);
    }
    else {
        fprintf(logFile, "\n [%s] [%s] %s %s %s sama dengan %s", timestamp, type, input1, operation, input2, result);
    }

    fclose(logFile);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <operation>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *operation = argv[1];
    int pipefd[2];
    pid_t pid;
    char input2[MAX_LENGTH], input1[MAX_LENGTH];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { 
        close(pipefd[1]); 

        int result;
        read(pipefd[0], &result, sizeof(result));
        close(pipefd[0]);

    } else {
        close(pipefd[0]); 

        char input1[MAX_LENGTH], input2[MAX_LENGTH];
        int num1, num2, result;

        printf("Masukkan dua angka (dalam kata): ");
        scanf("%s %s", input1, input2);

        convertToNumber(input1, &num1);
        convertToNumber(input2, &num2);

        if (strcmp(operation, "-kali") == 0) {
            result = num1 * num2;
        } else if (strcmp(operation, "-tambah") == 0) {
            result = num1 + num2;
        } else if (strcmp(operation, "-kurang") == 0) {
            result = num1 - num2;
            if (result < 0) {
                result = -1;
            }
        } else if (strcmp(operation, "-bagi") == 0) {
            if (num2 == 0) {
                fprintf(stderr, "Error: Pembagian dengan nol tidak diperbolehkan.\n");
                exit(EXIT_FAILURE);
            }
            result = num1 / num2;
        } else {
            fprintf(stderr, "Operasi tidak valid.\n");
            exit(EXIT_FAILURE);
        }

        char message[MAX_LENGTH];
        switch (result) {
            case -1:
                strcpy(message, "ERROR");
                break;
            default:
                convertToWords(result, message);
        }

        printf("Hasil %s adalah %s.\n", operation, message);
        writeToLog(operation, input1, input2, operation, message);
        exit(EXIT_SUCCESS);

        write(pipefd[1], &result, sizeof(result));
        close(pipefd[1]);

        wait(NULL);
    }

    return 0;
}
```
#### - Header dan Definisi Konstanta
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>

#define MAX_LENGTH 20
```

Header files digunakan untuk mengakses fungsi-fungsi standar yang diperlukan dalam program:
stdio.h: Untuk fungsi input-output standar seperti printf dan scanf.
stdlib.h: Untuk fungsi-fungsi umum seperti exit.
unistd.h, sys/types.h, sys/wait.h: Untuk fungsi-fungsi sistem seperti fork, pipe, wait.

Konstanta MAX_LENGTH ditetapkan sebagai panjang maksimum string yang digunakan untuk representasi kata-kata bilangan. Ini membantu dalam alokasi memori dan pembatasan panjang input yang dapat diterima.

#### - Fungsi-fungsi untuk menjalankan operasi aritmetika sesuai permintaan soal.

```c
void convertToNumber(char *str, int *num) {
    if (strcmp(str, "nol") == 0) {
        *num = 0;
    } else if (strcmp(str, "satu") == 0) {
        *num = 1;
    } else if (strcmp(str, "dua") == 0) {
        *num = 2;
    } else if (strcmp(str, "tiga") == 0) {
        *num = 3;
    } else if (strcmp(str, "empat") == 0) {
        *num = 4;
    } else if (strcmp(str, "lima") == 0) {
        *num = 5;
    } else if (strcmp(str, "enam") == 0) {
        *num = 6;
    } else if (strcmp(str, "tujuh") == 0) {
        *num = 7;
    } else if (strcmp(str, "delapan") == 0) {
        *num = 8;
    } else if (strcmp(str, "sembilan") == 0) {
        *num = 9;
    }
}
```

>Fungsi untuk mengubah kata-kata menjadi angka.

```c
void convertToWords(int num, char *str) {
    char *ones[] = {"nol", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    char *teens[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    char *tens[] = {"", "sepuluh", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};

    if (num >= 0 && num <= 9) {
        strcpy(str, ones[num]);
    } else if (num >= 10 && num <= 19) {
        strcpy(str, teens[num - 10]);
    } else if (num >= 20 && num <= 99) {
        strcpy(str, tens[num / 10]);
        if (num % 10 != 0) {
            strcat(str, " ");
            strcat(str, ones[num % 10]);
        }
    }
}
```

>Fungsi untuk mengubah lagi dari kata-kata menjadi angka.

```c
void writeToLog(char *type, char *input1, char *input2, char *operation, char *result) {
    time_t now;
    struct tm *local;
    char timestamp[20];
    FILE *logFile;

    time(&now);
    local = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%y/%m/%d %H:%M:%S", local);

    logFile = fopen("histori.log", "a");
    if (logFile == NULL) {
        perror("Failed to open histori.log");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; type[i] != '\0'; i++) {
        type[i] = toupper(type[i]);
    }

    if (strcmp(result, "ERROR") == 0) {
        fprintf(logFile, "\n [%s] [%s] %s pada pengurangan", timestamp, type, result);
    }
    else {
        fprintf(logFile, "\n [%s] [%s] %s %s %s sama dengan %s", timestamp, type, input1, operation, input2, result);
    }

    fclose(logFile);
}
```

>Fungsi untuk menuliskan hasil operasi ke dalam file "histori.log" dengan format yang sudah ditentukan.

#### - Fungsi main.

```c
if (argc != 2) {
        fprintf(stderr, "Usage: %s <operation>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
```

>Fungsi main dimulai dengan memeriksa jumlah argumen yang diberikan saat menjalankan program. Program ini diharapkan menerima dua argumen: nama program itu sendiri (argv[0]) dan operasi matematika yang akan dilakukan. Jika jumlah argumen tidak sesuai, program akan menampilkan pesan kesalahan dan keluar dengan status kesalahan.

```c
int pipefd[2];
if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
```

>Selanjutnya, program membuat sebuah pipe dengan menggunakan fungsi pipe(). Pipe ini akan digunakan untuk komunikasi antara proses induk dan proses anak. Jika pembuatan pipe gagal, program akan menampilkan pesan kesalahan dan keluar dengan status kesalahan.

```c
if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
```

>Program selanjutnya membuat proses anak dengan menggunakan fungsi fork(). Jika pembuatan proses anak gagal, program akan menampilkan pesan kesalahan dan keluar dengan status kesalahan.

```c
if (pid == 0) { 
        close(pipefd[1]); 

        int result;
        read(pipefd[0], &result, sizeof(result));
        close(pipefd[0]);

    } 
```

>Jika proses yang berjalan adalah proses anak (pid == 0), proses tersebut akan menutup ujung tulis pipe (pipefd[1]) karena proses anak hanya akan membaca dari pipe.

```c
else {
        close(pipefd[0]); 

        char input1[MAX_LENGTH], input2[MAX_LENGTH];
        int num1, num2, result;

        printf("Masukkan dua angka (dalam kata): ");
        scanf("%s %s", input1, input2);

        convertToNumber(input1, &num1);
        convertToNumber(input2, &num2);
```

>Jika proses yang berjalan adalah proses induk (pid != 0), proses tersebut akan menutup ujung baca pipe (pipefd[0]) karena proses induk hanya akan menulis ke pipe.

```
char input1[MAX_LENGTH], input2[MAX_LENGTH];c
int num1, num2, result;
printf("Masukkan dua angka (dalam kata): ");
scanf("%s %s", input1, input2);
```

>Proses induk akan meminta pengguna untuk memasukkan dua bilangan dalam bentuk kata-kata. Input ini disimpan dalam dua variabel input1 dan input2.

```
 convertToNumber(input1, &num1);c
        convertToNumber(input2, &num2);
```

>Selanjutnya, program akan menggunakan fungsi convertToNumber untuk mengonversi input kata-kata bilangan menjadi angka integer (num1 dan num2). Setelah itu, program akan melakukan operasi aritmatika sesuai dengan argumen yang diberikan.

```
printf("Hasil %s adalah %s.\n", operation, message);
writeToLog(operation, input1, input2, operation, message);
```

>Selanjutnya, program akan menggunakan fungsi convertToNumber untuk mengonversi input kata-kata bilangan menjadi angka integer (num1 dan num2). Setelah itu, program akan melakukan operasi aritmatika sesuai dengan argumen yang diberikan.

```
printf("Hasil %s adalah %s.\n", operation, message);
writeToLog(operation, input1, input2, operation, message);
```

>Setelah hasil operasi diperoleh, program akan menampilkan hasilnya ke layar dan menulis informasi operasi tersebut ke file log menggunakan fungsi writeToLog.

```
write(pipefd[1], &result, sizeof(result));
close(pipefd[1]);
wait(NULL);
```

>Setelah menulis hasil operasi ke pipe (pipefd[1]), proses induk akan menutup ujung tulis pipe dan kemudian menunggu proses anak selesai (wait(NULL)). Ini memastikan bahwa proses induk menunggu hingga proses anak selesai sebelum program selesai berjalan.

### Dokumentasi

![WhatsApp Image 2024-05-11 at 02 05 09 (1)](https://github.com/Gandhiert/Sisop-3-2024-MH-IT05/assets/142889150/4154c328-a544-45a7-81ac-97c4add8b5d9)
>Contoh penggunaan operasi aritmetika.

![WhatsApp Image 2024-05-11 at 02 05 09](https://github.com/Gandhiert/Sisop-3-2024-MH-IT05/assets/142889150/62e17f75-35f4-465a-ad14-c72faf2dcb25)
>Contoh hasil dari histori.log.


