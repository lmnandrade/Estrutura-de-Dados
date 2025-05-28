#include <stdio.h>
#include <stdlib.h>

#define MAX_LEN 32

int main() {
    FILE *f = fopen("data.dat", "rb");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    char date[MAX_LEN];
    char time_utc[MAX_LEN];
    char city[MAX_LEN];
    char country[MAX_LEN];

    float latitude;
    float longitude;
    float magnitude;
    float depth;
    float impact_score;

    while (fread(date, sizeof(char), MAX_LEN, f) == MAX_LEN &&
           fread(time_utc, sizeof(char), MAX_LEN, f) == MAX_LEN &&
           fread(city, sizeof(char), MAX_LEN, f) == MAX_LEN &&
           fread(country, sizeof(char), MAX_LEN, f) == MAX_LEN &&
           fread(&latitude, sizeof(float), 1, f) == 1 &&
           fread(&longitude, sizeof(float), 1, f) == 1 &&
           fread(&magnitude, sizeof(float), 1, f) == 1 &&
           fread(&depth, sizeof(float), 1, f) == 1 &&
           fread(&impact_score, sizeof(float), 1, f) == 1) {

        // Ensure strings are null-terminated
        date[MAX_LEN - 1] = '\0';
        time_utc[MAX_LEN - 1] = '\0';
        city[MAX_LEN - 1] = '\0';
        country[MAX_LEN - 1] = '\0';

    }

    fclose(f);
    return 0;
}