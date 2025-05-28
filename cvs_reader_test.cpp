#include "csv_reader.h" // Inclui a declaração da função e da estrutura
#include <iostream>     // Para std::cerr
#include <fstream>      // Para std::ifstream
#include <sstream>      // Para std::stringstream

// Implementação da função lerCSV com filtragem por data
std::vector<Registro> lerCSV(const std::string& nomearqui, const std::string& dataDesejada) {
    std::vector<Registro> registros; // Vetor para armazenar os registros lidos
    std::ifstream file(nomearqui);   // Abre o arquivo para leitura

    // Verifica se o arquivo foi aberto com sucesso
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file '" << nomearqui << "'" << std::endl;
        return registros; // Retorna um vetor vazio em caso de erro
    }

    std::string linha;
    // Lê e ignora a primeira linha (cabeçalho do CSV)
    if (!std::getline(file, linha)) {
        std::cerr << "Error: Could not read header line from '" << nomearqui << "'" << std::endl;
        return registros;
    }

    // Loop principal para ler cada linha do arquivo após o cabeçalho
    while (std::getline(file, linha)) {
        std::stringstream ss(linha); // Cria um stringstream a partir da linha lida
        Registro registro;           // Cria um novo objeto Registro para preencher

        // Usa std::getline com o delimitador ',' para extrair cada campo
        if (std::getline(ss, registro.date, ',') &&
            std::getline(ss, registro.time, ',') &&
            std::getline(ss, registro.city, ',') &&
            std::getline(ss, registro.ctry, ',') &&
            std::getline(ss, registro.lat, ',') &&
            std::getline(ss, registro.lon, ',') &&
            std::getline(ss, registro.mag, ',') &&
            std::getline(ss, registro.depth, ',') &&
            std::getline(ss, registro.imps)) {
            
            // Adiciona apenas registros que correspondem à data desejada
            if (registro.date == dataDesejada) {
                registros.push_back(registro);
            }
        } else {
            std::cerr << "Warning: Skipping malformed line: " << linha << std::endl;
        }
    }

    file.close(); // Fecha o arquivo
    return registros; // Retorna o vetor contendo os registros filtrados
}