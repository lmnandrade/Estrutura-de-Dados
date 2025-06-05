#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <string>
#include <vector>
#include <list> // Para encadeamento
#include <functional> // Para std::hash
#include "csv_reader.h" // Para EarthquakeRecord

// Estrutura para armazenar o par chave-valor na tabela hash
struct HashNode {
    std::string key;
    EarthquakeRecord record;

    HashNode(std::string k, const EarthquakeRecord& r) : key(k), record(r) {}
};

class HashTable {
private:
    std::vector<std::list<HashNode>> table;
    size_t num_elements;
    size_t table_size;

    // Função hash privada
    size_t hash_function(const std::string& key) const;

    // Função para gerar a chave a partir do registro
    std::string generate_key(const EarthquakeRecord& record) const;

    // Redimensionar a tabela quando o fator de carga aumenta muito
    void rehash();

public:
    explicit HashTable(size_t initial_size = 100); // Construtor

    void insert_record(const EarthquakeRecord& record);
    const EarthquakeRecord* search_record(const std::string& date, const std::string& time_utc, const std::string& city)const;
    bool remove_record(const std::string& date, const std::string& time_utc, const std::string& city);

    int get_count() const;
    bool is_empty() const;
    void display_table_distribution() const; // Para análise da distribuição
    std::vector<EarthquakeRecord> get_all_records_vector() const; // Para consistência com outras estruturas
     void list_all_records() const; // Para consistência com outras estruturas
     void list_by_min_magnitude(const std::string& min_mag_str) const; // Para consistência
     void list_by_year(const std::string& year_str) const; // Para consistência
     // Você pode adicionar mais funções de listagem/busca se necessário
};

#endif // HASH_TABLE_H