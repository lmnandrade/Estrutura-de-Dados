#ifndef CSV_READER_H
#define CSV_READER_H

#include <string>
#include <vector>
#include <iostream> // Para std::cerr e std::cout

// Estrutura para armazenar os dados de um registro de terremoto
// Todos os campos são lidos como strings. A conversão para tipos numéricos
// será feita posteriormente, conforme necessário.
struct EarthquakeRecord {
    std::string date;
    std::string time;
    std::string city;
    std::string country;
    std::string latitude;
    std::string longitude;
    std::string magnitude;
    std::string depth;
    std::string impact_score;

    // Construtor padrão (opcional, mas útil para garantir que as strings estejam vazias)
    EarthquakeRecord() = default; // Usa o construtor padrão gerado pelo compilador
};

// Função para ler o arquivo CSV e retornar um vetor de EarthquakeRecord
// Retorna um vetor vazio se o arquivo não puder ser aberto ou se houver erros críticos.
std::vector<EarthquakeRecord> read_earthquake_csv(const std::string& filename);

// Função para exibir um único registro de terremoto de forma formatada
void displayRecord(const EarthquakeRecord& record);
void save_earthquake_csv(const std::string& filename, const std::vector<EarthquakeRecord>& records);

#endif // CSV_READER_H