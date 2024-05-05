// actions.c
#include <stdio.h>
#include <string.h>

char* gap(float distance) {
    if (distance < 3.5) {
        return "Gogogo";
    } else if (distance >= 3.5 && distance <= 10) {
        return "Push";
    } else {
        return "Stay out of trouble";
    }
}

char* fuel(float percentage) {
    if (percentage > 80) {
        return "Push Push Push";
    } else if (percentage >= 50 && percentage <= 80) {
        return "You can go";
    } else {
        return "Conserve Fuel";
    }
}

char* tire(int wear) {
    if (wear > 80) {
        return "Go Push Go Push";
    } else if (wear >= 50 && wear <= 80) {
        return "Good Tire Wear";
    } else if (wear >= 30 && wear < 50) {
        return "Conserve Your Tire";
    } else {
        return "Box Box Box";
    }
}

char* tireChange(char* tireType) {
    if (strcmp(tireType, "Soft") == 0) {
        return "Mediums Ready";
    } else if (strcmp(tireType, "Medium") == 0) {
        return "Box for Softs";
    }
    return "Tidak Ada/Diketahui";
}
