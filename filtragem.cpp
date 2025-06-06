#include "filtragem.h"
#include <iostream>
#include <algorithm> // Para std::all_of

// Função para verificar se uma string é considerada "vazia" ou "nula" para nossos propósitos
// Duplicada aqui para manter o módulo independente, mas idealmente estaria em um utils.h
bool is_field_empty_for_sanitization(const std::string& field) {
    if (field.empty()) {
        return true;
    }
    // Verifica se a string contém apenas espaços em branco
    return std::all_of(field.begin(), field.end(), ::isspace);
}

std::vector<EarthquakeRecord> sanitize_earthquake_records(
    const std::vector<EarthquakeRecord>& raw_records,
    int& removed_count,
    bool verbose)
{
    std::vector<EarthquakeRecord> sanitized_records;
    sanitized_records.reserve(raw_records.size()); // Otimização: pré-aloca memória

    removed_count = 0;
    int record_index = 0; // Para logs, se necessário

    for (const auto& record : raw_records) {
        record_index++;
        bool is_valid_record = true;
        std::string missing_fields_info = "";

        // Defina aqui quais campos são OBRIGATÓRIOS
        if (is_field_empty_for_sanitization(record.date)) { is_valid_record = false; missing_fields_info += "date, "; }
        if (is_field_empty_for_sanitization(record.latitude)) { is_valid_record = false; missing_fields_info += "latitude, "; }
        if (is_field_empty_for_sanitization(record.longitude)) { is_valid_record = false; missing_fields_info += "longitude, "; }
        if (is_field_empty_for_sanitization(record.depth)) { is_valid_record = false; missing_fields_info += "depth, "; }
        if (is_field_empty_for_sanitization(record.magnitude)) { is_valid_record = false; missing_fields_info += "magnitude, "; }
        if (is_field_empty_for_sanitization(record.country)) { is_valid_record = false; missing_fields_info += "country, "; }
        // O campo 'time' (record.time) não está sendo verificado aqui, conforme solicitado.
        // Adicione outros campos obrigatórios se necessário:
        // if (is_field_empty_for_sanitization(record.city)) { is_valid_record = false; missing_fields_info += "city, "; }
        // if (is_field_empty_for_sanitization(record.impact_score)) { is_valid_record = false; missing_fields_info += "impact_score, "; }

        if (is_valid_record) {
            sanitized_records.push_back(record);
        } else {
            removed_count++;
            if (verbose && removed_count <= 10) { // Mostra aviso para os primeiros 10 descartados
                 // Usar record.id se disponível e confiável, ou um índice
                std::string record_identifier = "Indice " + std::to_string(record_index - 1); // -1 porque record_index é 1-based
                std::cout << "AVISO (Sanitizacao - Registro " << record_identifier << "): Descartado devido a campos essenciais vazios/nulos: "
                          << (missing_fields_info.empty() ? "" : missing_fields_info.substr(0, missing_fields_info.length() - 2))
                          << std::endl;
            }
        }
    }
    if (verbose && removed_count > 10) {
        std::cout << "AVISO (Sanitizacao): ... e mais " << (removed_count - 10) << " registros foram descartados (total: " << removed_count << ")." << std::endl;
    }


    return sanitized_records;
}