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