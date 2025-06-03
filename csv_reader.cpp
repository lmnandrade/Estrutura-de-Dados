/*
#include "csv_reader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> // Para std::remove_if

// Função auxiliar para remover espaços em branco extras de uma string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::vector<Registro> lerCSV(const std::string& nomearqui) {
    std::vector<Registro> registros;
    std::ifstream file(nomearqui);

    if (!file.is_open()) {
        std::cerr << "Erro: Não foi possível abrir o arquivo '" << nomearqui << "'\n";
        return registros;
    }

    std::string linha;
    // Lê o cabeçalho
    if (!std::getline(file, linha)) {
        std::cerr << "Erro: Não foi possível ler o cabeçalho de '" << nomearqui << "'\n";
        return registros;
    }

    // Lê cada linha do arquivo
    int linha_num = 1;
    while (std::getline(file, linha)) {
        linha_num++;
        if (linha.empty()) continue; // Pula linhas em branco

        std::stringstream ss(linha);
        Registro registro;

        // Lê cada campo, respeitando a ordem do CSV
        if (std::getline(ss, registro.date, ',') &&
            std::getline(ss, registro.time, ',') &&
            std::getline(ss, registro.city, ',') &&
            std::getline(ss, registro.ctry, ',') &&
            std::getline(ss, registro.lat, ',') &&
            std::getline(ss, registro.lon, ',') &&
            std::getline(ss, registro.mag, ',') &&
            std::getline(ss, registro.depth, ',') &&
            std::getline(ss, registro.impactScore)) {

            // Remove espaços em branco extras dos campos
            registro.date = trim(registro.date);
            registro.time = trim(registro.time);
            registro.city = trim(registro.city);
            registro.ctry = trim(registro.ctry);
            registro.lat = trim(registro.lat);
            registro.lon = trim(registro.lon);
            registro.mag = trim(registro.mag);
            registro.depth = trim(registro.depth);
            registro.impactScore = trim(registro.impactScore);

            registros.push_back(registro);
        } else {
            std::cerr << "Aviso: Linha malformada ignorada (" << linha_num << "): " << linha << std::endl;
        }
    }

    file.close();
    return registros;
} */

#include "csv_reader.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Função para dividir uma string por um delimitador
std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// Função para ler o arquivo CSV e retornar um vetor de registros
std::vector<EarthquakeRecord> readEarthquakeCSV(const std::string& filename) {
    std::vector<EarthquakeRecord> records;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return records;
    }
    
    std::string line;
    
    // Ler a primeira linha (cabeçalho) e descartá-la
    std::getline(file, line);
    
    // Ler as linhas de dados
    while (std::getline(file, line)) {
        std::vector<std::string> fields = splitString(line, ',');
        
        // Verificar se a linha tem o número correto de campos
        if (fields.size() >= 9) {
            EarthquakeRecord record;
            record.date = fields[0];
            record.time = fields[1];
            record.city = fields[2];
            record.country = fields[3];
            record.latitude = fields[4];
            record.longitude = fields[5];
            record.magnitude = fields[6];
            record.depth = fields[7];
            record.impactScore = fields[8];
            
            records.push_back(record);
        } else {
            std::cerr << "Linha com formato incorreto: " << line << std::endl;
        }
    }
    
    file.close();
    std::cout << "Total de registros lidos: " << records.size() << std::endl;
    return records;
}

// Função para exibir um registro
void displayRecord(const EarthquakeRecord& record) {
    std::cout << "Data: " << record.date << std::endl;
    std::cout << "Hora (UTC): " << record.time << std::endl;
    std::cout << "Cidade: " << record.city << std::endl;
    std::cout << "País: " << record.country << std::endl;
    std::cout << "Latitude: " << record.latitude << std::endl;
    std::cout << "Longitude: " << record.longitude << std::endl;
    std::cout << "Magnitude: " << record.magnitude << std::endl;
    std::cout << "Profundidade (km): " << record.depth << std::endl;
    std::cout << "Pontuação de Impacto: " << record.impactScore << std::endl;
    std::cout << "------------------------" << std::endl;
}

// Função para exibir todos os registros
void displayAllRecords(const std::vector<EarthquakeRecord>& records) {
    std::cout << "Total de registros: " << records.size() << std::endl;
    
    for (size_t i = 0; i < records.size(); i++) {
        std::cout << "Registro #" << (i + 1) << std::endl;
        displayRecord(records[i]);
    }
}