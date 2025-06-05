#include "hash_table.h"
#include <iostream>
#include <iomanip> // Para std::setprecision e std::fixed
#include <functional> // Para std::hash já incluído em hash_table.h, mas bom ter aqui para clareza se necessário.

// Namespace KDTreeHelpers (para try_string_to_double)
// Assumindo que está definido em outro lugar e acessível, ou coloque a definição aqui
// se não estiver em um header compartilhado.
// Se estiver em kd_tree.h e kd_tree.h for incluído por hash_table.h (via csv_reader.h, por exemplo),
// então pode não ser necessário repetir aqui.
// Por segurança, se a definição de KDTreeHelpers não estiver em um header incluído por hash_table.h,
// você precisaria dela aqui ou em um utils.h.
// Para este exemplo, vamos assumir que `KDTreeHelpers::try_string_to_double` está acessível.
// Se você copiou o namespace KDTreeHelpers para kd_tree.cpp, e hash_table.cpp não inclui kd_tree.cpp (o que não deveria),
// então você precisaria da definição de KDTreeHelpers aqui também ou em um header comum.
// Por enquanto, vou assumir que está disponível através das inclusões de hash_table.h
namespace KDTreeHelpers {
    // Esta é uma cópia da definição que estava em kd_tree.cpp.
    // Se você criar um utils.h, seria melhor colocá-la lá e incluir utils.h aqui e em kd_tree.cpp.
    inline bool try_string_to_double(const std::string& s, double& out, const std::string& field_name_for_error = "", bool suppress_error_message = false) {
        try {
            out = std::stod(s);
            return true;
        } catch (const std::invalid_argument& ia) {
            if (!suppress_error_message && !field_name_for_error.empty()) {
                std::cerr << "ERRO: Valor invalido para " << field_name_for_error << ": '" << s << "' (nao e um numero)." << std::endl;
            } else if (!suppress_error_message) {
                 std::cerr << "ERRO: Valor invalido: '" << s << "' (nao e um numero)." << std::endl;
            }
            return false;
        } catch (const std::out_of_range& oor) {
            if (!suppress_error_message && !field_name_for_error.empty()) {
                std::cerr << "ERRO: Valor fora do intervalo para " << field_name_for_error << ": '" << s << "'." << std::endl;
            } else if (!suppress_error_message) {
                 std::cerr << "ERRO: Valor fora do intervalo: '" << s << "'." << std::endl;
            }
            return false;
        }
    }
} // namespace KDTreeHelpers


// --- Implementação da Chave e Função Hash ---

std::string HashTable::generate_key(const EarthquakeRecord& record) const {
    // Normalizar a cidade para minúsculas pode ajudar a evitar chaves diferentes para a mesma cidade com caixa diferente.
    // Exemplo simples (pode ser mais robusto com locales):
    // std::string city_lower = record.city;
    // std::transform(city_lower.begin(), city_lower.end(), city_lower.begin(), ::tolower);
    // return record.date + "_" + record.time + "_" + city_lower;
    return record.date + "_" + record.time + "_" + record.city;
}

size_t HashTable::hash_function(const std::string& key) const {
    std::hash<std::string> hasher;
    return hasher(key) % table_size;
}

// --- Construtor e Rehash ---
HashTable::HashTable(size_t initial_size) : num_elements(0), table_size(initial_size) {
    if (table_size == 0) { // Garante que o tamanho da tabela não seja zero
        table_size = 100; // Um tamanho padrão razoável
    }
    table.resize(table_size);
}

void HashTable::rehash() {
    size_t old_table_size = table_size;
    std::vector<std::list<HashNode>> old_table = table;

    table_size *= 2; 
    if (table_size == 0) table_size = 2; // Caso extremo se começou com 0 e foi para 0
    table.assign(table_size, std::list<HashNode>()); 
    num_elements = 0; 

    // std::cout << "INFO: Realizando rehash da tabela de " << old_table_size << " para " << table_size << " posicoes." << std::endl;

    for (size_t i = 0; i < old_table_size; ++i) {
        for (const auto& node : old_table[i]) {
            size_t new_index = hash_function(node.key);
            table[new_index].push_back(node); 
            num_elements++;
        }
    }
}

// --- Operações Públicas ---
void HashTable::insert_record(const EarthquakeRecord& record) {
    const double MAX_LOAD_FACTOR = 0.75;
    if (table_size == 0 || static_cast<double>(num_elements + 1) / table_size >= MAX_LOAD_FACTOR) {
        rehash();
    }

    std::string key = generate_key(record);
    size_t index = hash_function(key);

    for (const auto& node : table[index]) {
        if (node.key == key) {
            // std::cout << "AVISO (HashTable): Chave "" << key << "" ja existe. Registro nao inserido para evitar duplicata." << std::endl;
            return; 
        }
    }

    table[index].emplace_back(key, record);
    num_elements++;
}

const EarthquakeRecord* HashTable::search_record(const std::string& date, const std::string& time_utc, const std::string& city) const {
    EarthquakeRecord temp_record; 
    temp_record.date = date;
    temp_record.time = time_utc;
    temp_record.city = city;
    std::string key = generate_key(temp_record);

    if (table_size == 0) return nullptr; // Evita divisão por zero se table_size for 0
    size_t index = hash_function(key);

    for (const auto& node : table[index]) {
        if (node.key == key) {
            return &(node.record); 
        }
    }
    return nullptr; 
}

bool HashTable::remove_record(const std::string& date, const std::string& time_utc, const std::string& city){
    EarthquakeRecord temp_record;
    temp_record.date = date;
    temp_record.time = time_utc;
    temp_record.city = city;
    std::string key = generate_key(temp_record);

    if (table_size == 0) return false;
    size_t index = hash_function(key);

    for (auto it = table[index].begin(); it != table[index].end(); ++it) {
        if (it->key == key) {
            table[index].erase(it);
            num_elements--;
            return true;
        }
    }
    return false; 
}

int HashTable::get_count() const {
    return num_elements;
}

bool HashTable::is_empty() const {
    return num_elements == 0;
}

void HashTable::display_table_distribution() const {
    std::cout << "\n--- Distribuicao da Tabela Hash ---" << std::endl;
    if (table_size == 0) {
        std::cout << "Tabela Hash nao inicializada ou com tamanho zero." << std::endl;
        return;
    }
    std::cout << "Tamanho da Tabela: " << table_size << std::endl;
    std::cout << "Numero de Elementos: " << num_elements << std::endl;
    std::cout << "Fator de Carga: " << std::fixed << std::setprecision(2) << static_cast<double>(num_elements) / table_size << std::endl;
    
    int non_empty_buckets = 0;
    for (size_t i = 0; i < table_size; ++i) {
        if (!table[i].empty()) {
            non_empty_buckets++;
            std::cout << "Indice [" << std::setw(3) << i << "]: " << std::setw(2) << table[i].size() << " elementos (Chaves: ";
            bool first_key = true;
            int keys_shown = 0;
            for(const auto& node : table[i]) {
                if (!first_key) std::cout << ", ";
                // Mostra uma parte da chave para identificação, corrigido com "
                std::cout << "\"";
                if (node.key.length() > 15) {
                    std::cout << node.key.substr(0, 12) << "...";
                } else {
                    std::cout << node.key;
                }
                std::cout << "\"";
                first_key = false;
                keys_shown++;
                if (keys_shown >= 3 && table[i].size() > 3) { // Limita o número de chaves mostradas por bucket
                    std::cout << ", ...";
                    break;
                }
            }
            std::cout << ")" << std::endl;
        }
    }
    if (non_empty_buckets == 0 && num_elements > 0) {
         std::cout << "AVISO: Elementos existem, mas nenhum bucket populado. Verifique a funcao hash ou rehash." << std::endl;
    } else if (non_empty_buckets == 0 && num_elements == 0) {
         std::cout << "Tabela vazia." << std::endl;
    }
    std::cout << "----------------------------------" << std::endl;
}

std::vector<EarthquakeRecord> HashTable::get_all_records_vector() const {
    std::vector<EarthquakeRecord> all_records;
    all_records.reserve(num_elements);
    for (const auto& bucket : table) {
        for (const auto& node : bucket) {
            all_records.push_back(node.record);
        }
    }
    return all_records;
}

void HashTable::list_all_records() const {
    if (is_empty()) {
        std::cout << "A Tabela Hash esta vazia." << std::endl;
        return;
    }
    std::cout << "\n--- Todos os Registros de Terremotos na Tabela Hash (" << num_elements << ") ---" << std::endl;
    int record_num = 1;
    for (const auto& bucket : table) {
        for (const auto& node : bucket) {
            // CORRIGIDO: Usar " para aspas literais
            std::cout << "Registro #" << record_num++ << " (Chave: "" << node.key << ""):" << std::endl;
            std::cout << "  ";
            displayRecord(node.record); 
            if (num_elements > 1 && record_num <= num_elements) { // Evita linha extra no final
                 std::cout << "  --------------------------------------------------------------------------------" << std::endl;
            }
        }
    }
    if (num_elements > 0) { // Linha final se houver registros
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}

void HashTable::list_by_min_magnitude(const std::string& min_mag_str) const {
    if (is_empty()) {
        std::cout << "A Tabela Hash esta vazia. Nada para listar." << std::endl;
        return;
    }
    double min_mag_val;
    if (!KDTreeHelpers::try_string_to_double(min_mag_str, min_mag_val, "Magnitude Minima", false)) {
        return; 
    }
    std::cout << "\n--- Registros na Tabela Hash com Magnitude >= " << std::fixed << std::setprecision(1) << min_mag_val << " ---" << std::endl;
    int total_found = 0;
    int record_num_overall = 0; // Para a linha separadora
    for (const auto& bucket : table) {
        for (const auto& node : bucket) {
            record_num_overall++;
            double current_rec_mag;
            if (KDTreeHelpers::try_string_to_double(node.record.magnitude, current_rec_mag, "", true)) {
                if (current_rec_mag >= min_mag_val) {
                    total_found++;
                    // CORRIGIDO: Usar " para aspas literais
                    std::cout << "Registro Filtrado #" << total_found << " (Chave: "" << node.key << ""):" << std::endl;
                    std::cout << "  "; displayRecord(node.record);
                    // Adiciona linha separadora se não for o último elemento filtrado E se houver mais elementos na tabela geral
                    if (total_found > 0 && record_num_overall < num_elements) {
                         std::cout << "  --------------------------------------------------------------------------------" << std::endl;
                    }
                }
            }
        }
    }
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado com magnitude >= " << min_mag_str << "." << std::endl;
    }
    // Linha final se houver registros filtrados
    if (total_found > 0) {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}

void HashTable::list_by_year(const std::string& year_str) const {
    if (is_empty()) {
        std::cout << "A Tabela Hash esta vazia. Nada para listar." << std::endl;
        return;
    }
    if (year_str.length() != 4) {
        std::cerr << "Erro: Formato de ano invalido. Use AAAA (ex: 2023)." << std::endl;
        return;
    }
    try { std::stoi(year_str); } 
    catch (const std::exception&) {
        std::cerr << "Erro: Ano invalido: "" << year_str << """ << std::endl; // Corrigido para aspas
        return;
    }

    std::cout << "\n--- Registros na Tabela Hash do Ano " << year_str << " ---" << std::endl;
    int total_found = 0;
    int record_num_overall = 0;
    for (const auto& bucket : table) {
        for (const auto& node : bucket) {
            record_num_overall++;
            if (node.record.date.rfind(year_str, 0) == 0) { 
                total_found++;
                // CORRIGIDO: Usar " para aspas literais
                std::cout << "Registro Filtrado #" << total_found << " (Chave: "" << node.key << ""):" << std::endl;
                std::cout << "  "; displayRecord(node.record);
                 if (total_found > 0 && record_num_overall < num_elements) {
                     std::cout << "  --------------------------------------------------------------------------------" << std::endl;
                }
            }
        }
    }
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado para o ano " << year_str << "." << std::endl;
    }
    if (total_found > 0) {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}