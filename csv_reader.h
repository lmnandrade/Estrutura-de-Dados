#ifndef CSV_READER_H
#define CSV_READER_H

#include <vector>
#include <string>

// Estrutura para armazenar cada registro de terremoto
struct EarthquakeRecord {
    std::string date;
    std::string time;
    std::string city;
    std::string country;
    std::string latitude;
    std::string longitude;
    std::string magnitude;
    std::string depth;
    std::string impactScore;
};

// Função para ler o arquivo CSV e retornar um vetor de registros
std::vector<EarthquakeRecord> readEarthquakeCSV(const std::string& filename);

// Função para exibir um registro
void displayRecord(const EarthquakeRecord& record);

// Função para exibir todos os registros
void displayAllRecords(const std::vector<EarthquakeRecord>& records);

// Função auxiliar para dividir uma string por um delimitador
std::vector<std::string> splitString(const std::string& str, char delimiter);

#endif // CSV_READER_H