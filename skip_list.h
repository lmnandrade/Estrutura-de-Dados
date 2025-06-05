#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include "csv_reader.h" // Para EarthquakeRecord
#include <vector>
#include <string>
#include <memory> // Para std::unique_ptr ou std::shared_ptr se preferir
#include <random> // Para geração de níveis aleatórios
#include <limits> // Para std::numeric_limits

// --- Constantes ---
const int MAX_LEVEL_SKIP_LIST = 16; // Nível máximo para a skip list (pode ser ajustado)
const float P_SKIP_LIST = 0.5;      // Probabilidade de um nó subir um nível

// --- Estrutura do Nó da Skip List ---
struct SkipListNode {
    EarthquakeRecord record;
    std::vector<SkipListNode*> forward; // Vetor de ponteiros para os próximos nós em cada nível

    // Construtor
    SkipListNode(const EarthquakeRecord& rec, int level) : record(rec) {
        forward.resize(level + 1, nullptr); // Níveis de 0 a 'level'
    }

    // Para chave de comparação - usaremos uma combinação de data, hora e cidade
    // como um identificador único para ordenação e busca.
    // A latitude/longitude não são boas para ordenação primária aqui.
    std::string get_key() const {
        // Normalizar a cidade para minúsculas pode ajudar na consistência da chave
        // std::string city_lower = record.city;
        // std::transform(city_lower.begin(), city_lower.end(), city_lower.begin(), ::tolower);
        // return record.date + "_" + record.time + "_" + city_lower;
        return record.date + "_" + record.time + "_" + record.city;
    }
};

// --- Classe SkipList ---
class SkipList {
private:
    int current_max_level;      // Nível mais alto atualmente na lista
    SkipListNode* header;       // Nó cabeça (sentinela)
    int num_elements;           // Número de elementos na skip list

    std::mt19937 rng;           // Gerador de números aleatórios Mersenne Twister
    std::uniform_real_distribution<float> dist; // Distribuição para probabilidade

    // Função para gerar um nível aleatório para um novo nó
    int random_level();

public:
    SkipList();
    ~SkipList(); // Destrutor para liberar memória

    // Operações principais
    void insert_record(const EarthquakeRecord& record);
    EarthquakeRecord* search_record(const std::string& date, const std::string& time_utc, const std::string& city)const;
    bool remove_record(const std::string& date, const std::string& time_utc, const std::string& city); // Para consistência com HashTable
    // Adicionaremos uma versão de remoção que usa EarthquakeRecord se você preferir, ou pode adaptar a acima.
    // bool remove_record(const EarthquakeRecord& record_to_remove); // Alternativa


    // Funções auxiliares e de listagem (para consistência com outras estruturas)
    int get_count() const;
    bool is_empty() const;
    void list_all_records() const;
    void list_by_min_magnitude(const std::string& min_mag_str) const;
    void list_by_year(const std::string& year_str) const;
    std::vector<EarthquakeRecord> get_all_records_vector() const;

    // Funções de busca específicas que podem ser úteis para Skip List,
    // embora a chave primária seja data+hora+cidade.
    // Outras buscas (por cidade, magnitude exata, data) serão varreduras.
    std::vector<EarthquakeRecord> search_by_city(const std::string& city) const;
    std::vector<EarthquakeRecord> search_by_magnitude_exact(const std::string& magnitude) const;
    std::vector<EarthquakeRecord> search_by_date(const std::string& date) const;
    std::vector<EarthquakeRecord> search_by_city_and_magnitude(const std::string& city, const std::string& magnitude) const;

    // (Para consistência de assinatura com outras estruturas, embora não seja a chave primária da SL)
    // O método de remoção principal é por data, hora e cidade.
    // Este método pode ser uma varredura se necessário.
    bool remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country);


private:
    // Helper para converter string para double (pode estar em um utils.h)
    // Temporariamente definido aqui para auto-suficiência do exemplo
    // É melhor usar o KDTreeHelpers::try_string_to_double se já estiver acessível.
    // Por simplicidade, usarei um placeholder ou você pode copiar/incluir o try_string_to_double.
    // Vamos assumir que KDTreeHelpers está acessível via includes (ex: csv_reader.h -> kd_tree_helpers.h)
    // Se não, defina-o aqui ou em um utils.
};

#endif // SKIP_LIST_H