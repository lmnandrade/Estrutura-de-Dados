#include <stdio.h>
#include <stdlib.h>

#define COUNTRY_LEN 32

int main() {
    FILE *f = fopen("data.dat", "rb");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    float longitude;
    char country[COUNTRY_LEN];

    while (fread(&longitude, sizeof(float), 1, f) == 1 &&
           fread(country, sizeof(char), COUNTRY_LEN, f) == COUNTRY_LEN) {
        
        // Ensure string is null-terminated
        country[COUNTRY_LEN - 1] = '\0';

        printf("Longitude: %.2f, Country: %s\n", longitude, country);
    }

    fclose(f);
    return 0;
}
