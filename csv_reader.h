#ifndef CSV_READER_H // Garante que o conteúdo deste cabeçalho seja incluído apenas uma vez por arquivo de compilação
#define CSV_READER_H

#include <string>   // Para usar std::string
#include <vector>   // Para usar std::vector

// Define a estrutura para armazenar os dados de cada linha do CSV
struct Registro {
    std::string date;  // Data do evento
    std::string time;  // Hora do evento
    std::string city;  // Cidade onde ocorreu o evento
    std::string ctry;  // País onde ocorreu o evento
    std::string lat;   // Latitude
    std::string lon;   // Longitude
    std::string mag;   // Magnitude
    std::string depth; // Profundidade
    std::string imps;  // Impactos (ou outra métrica)
};

// Declaração da função que lê o arquivo CSV e retorna um vetor de Registros
// A função recebe o nome do arquivo como uma string constante por referência
std::vector<Registro> lerCSV(const std::string& nomearqui);

#endif // CSV_READER_H