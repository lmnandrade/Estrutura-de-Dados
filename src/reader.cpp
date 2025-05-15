int carregar_terremotos(const char *arquivo, Terremoto dados[], int max_amostras) {
    FILE *fp = fopen(arquivo, "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    char linha[MAX_LINHA];
    int count = 0;

    // Ignorar cabeçalho
    fgets(linha, MAX_LINHA, fp);

    while (fgets(linha, MAX_LINHA, fp) && count < max_amostras) {
        char *token;
        int coluna = 0;
        token = strtok(linha, ",");
        Terremoto t;

        while (token != NULL) {
            switch (coluna) {
                case 0: strncpy(t.date, token, sizeof(t.date)); break;
                case 1: strncpy(t.time_utc, token, sizeof(t.time_utc)); break;
                case 2: strncpy(t.city, token, sizeof(t.city)); break;
                case 3: strncpy(t.country, token, sizeof(t.country)); break;
                case 4: t.latitude = atof(token); break;
                case 5: t.longitude = atof(token); break;
                case 6: t.magnitude = atof(token); break;
                case 7: t.depth = atof(token); break;
                case 8: t.impact_score = atof(token); break;
                default: break;
            }
            token = strtok(NULL, ",");
            coluna++;
        }

        dados[count++] = t;
    }

    fclose(fp);
    return count;
}