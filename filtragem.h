#ifndef DATA_SANITIZER_H
#define DATA_SANITIZER_H

#include "csv_reader.h" // Para EarthquakeRecord
#include <vector>
#include <string>

// Função para verificar se um campo é considerado "vazio" ou "nulo"
bool is_field_empty_for_sanitization(const std::string& field);

// Função principal para limpar os registros, removendo aqueles com campos essenciais vazios.
// Retorna um novo vetor com os registros limpos.
// Opcionalmente, informa quantos registros foram removidos.
std::vector<EarthquakeRecord> sanitize_earthquake_records(
    const std::vector<EarthquakeRecord>& raw_records,
    int& removed_count,
    bool verbose = true
);

#endif // DATA_SANITIZER_H