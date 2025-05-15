#include "csv_reader.h" // Inclui a declaração da função e da estrutura
#include <iostream>     // Para std::cerr
#include <fstream>      // Para std::ifstream
#include <sstream>      // Para std::stringstream

// Implementação da função lerCSV
std::vector<Registro> lerCSV(const std::string& nomearqui) {
    std::vector<Registro> registros; // Vetor para armazenar os registros lidos
    std::ifstream file(nomearqui);   // Abre o arquivo para leitura

    // Verifica se o arquivo foi aberto com sucesso
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file '" << nomearqui << "'" << std::endl;
        // Em um cenário de biblioteca mais robusta, você poderia lançar uma exceção aqui.
        return registros; // Retorna um vetor vazio em caso de erro
    }

    std::string linha;
    // Lê e ignora a primeira linha (cabeçalho do CSV)
    // Verifica se a leitura da linha de cabeçalho foi bem-sucedida antes de continuar
    if (!std::getline(file, linha)) {
         std::cerr << "Error: Could not read header line from '" << nomearqui << "'" << std::endl;
         return registros; // Retorna vazio se não conseguir ler nem o cabeçalho
    }


    // Loop principal para ler cada linha do arquivo após o cabeçalho
    while (std::getline(file, linha)) {
        std::stringstream ss(linha); // Cria um stringstream a partir da linha lida
        Registro registro;           // Cria um novo objeto Registro para preencher

        // Usa std::getline com o delimitador ',' para extrair cada campo
        // É importante que a ordem das chamadas std::getline corresponda à ordem das colunas no CSV
        // e aos membros da struct Registro.
        if (std::getline(ss, registro.date, ',') &&
            std::getline(ss, registro.time, ',') &&
            std::getline(ss, registro.city, ',') &&
            std::getline(ss, registro.ctry, ',') &&
            std::getline(ss, registro.lat, ',') &&
            std::getline(ss, registro.lon, ',') &&
            std::getline(ss, registro.mag, ',') &&
            std::getline(ss, registro.depth, ',') &&
            // Para o último campo, não há delimitador ',' após ele na linha
            std::getline(ss, registro.imps)) { // Lê o restante da linha para o último campo
             registros.push_back(registro); // Adiciona o registro preenchido ao vetor
        } else {
            // Lida com linhas malformadas (que não têm o número esperado de colunas)
            std::cerr << "Warning: Skipping malformed line: " << linha << std::endl;
        }
    }

    file.close(); // Fecha o arquivo (opcional, o destrutor de ifstream faz isso)
    return registros; // Retorna o vetor contendo todos os registros lidos
}