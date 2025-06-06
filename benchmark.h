#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <vector>
#include <string>
#include <map>
#include <chrono> // Para std::chrono
#include <random> // Para std::mt19937
#include "csv_reader.h" // Para EarthquakeRecord

// Enum para os modos de restrição
enum class RestrictionMode {
    NONE,
    MAX_STRUCTURE_SIZE,
    SIMULATE_CONCURRENCY,
    SIMULATE_HIGH_LATENCY_IO,
    IRREGULAR_DATA_ARRIVAL
};

// Configuração para testes restritos
struct RestrictionConfig {
    RestrictionMode mode = RestrictionMode::NONE;
    std::string name_suffix = ""; // Ex: "_MaxSize10k"

    // Parâmetros específicos da restrição
    size_t max_elements = 0;
    std::chrono::milliseconds concurrency_yield_time = std::chrono::milliseconds(5);
    int concurrency_ops_per_yield = 20;
    std::chrono::milliseconds io_max_latency_ms = std::chrono::milliseconds(0);
    std::chrono::milliseconds arrival_max_interval_ms = std::chrono::milliseconds(0);
    
    std::mt19937* rng_engine = nullptr; // Deve ser fornecido externamente se necessário

    RestrictionConfig(RestrictionMode m = RestrictionMode::NONE, std::string suffix = "") 
        : mode(m), name_suffix(std::move(suffix)) {}
};

// Estrutura para armazenar as métricas de um benchmark
struct BenchmarkMetrics {
    long long insertion_time_ms = 0;
    double insertion_time_us_avg_element = 0.0;
    double search_existing_time_us_avg = 0.0;
    double search_non_existent_time_us_avg = 0.0;
    double removal_time_us_avg = 0.0;
    size_t memory_usage_bytes = 0;
    std::string collision_rate_info = "N/A";
    double avg_access_time_us = 0.0;
    double avg_latency_us_mixed_ops = 0.0;
    size_t actual_elements_processed_insertion = 0; // Elementos efetivamente inseridos
    size_t actual_elements_processed_ops = 0;       // Elementos usados para busca/remoção
};

class BenchmarkRunner {
public:
    BenchmarkRunner(const std::vector<EarthquakeRecord>& records, const std::string& benchmark_name);
    
    // Executa os benchmarks padrão E os restritos pré-configurados
    void run_all_benchmarks_and_restricted_tests(); 
    
    void save_results_to_csv(const std::string& filepath) const;

    std::vector<EarthquakeRecord> all_data_;
    std::string benchmark_name_;
    std::map<std::string, BenchmarkMetrics> results_;
    std::mt19937 rng_for_benchmarks_; // RNG para uso interno nos benchmarks

    // Função principal que chama os benchmarks padrão
    void run_standard_benchmarks(
        const std::vector<EarthquakeRecord>& data_for_insertion,
        const std::vector<EarthquakeRecord>& records_for_search_remove_ops,
        const std::vector<EarthquakeRecord>& non_existent_records_for_ops);
    
    // Testes de escalabilidade
    void run_scalability_tests();

    // Funções dedicadas para cada tipo de teste restrito
    void run_max_size_restriction_tests(size_t max_elements_limit);
    void run_concurrency_simulation_tests(std::chrono::milliseconds yield_time, int ops_per_yield);
    void run_high_latency_io_tests(std::chrono::milliseconds max_io_latency);
    void run_irregular_arrival_tests(std::chrono::milliseconds max_arrival_interval);
    void run_frequent_reindex_tests();

    // A função de benchmark genérica que aceita RestrictionConfig
    template<typename Structure>
    BenchmarkMetrics run_benchmark_for_structure(
        const std::string& structure_id_prefix,
        const std::vector<EarthquakeRecord>& data_for_insertion,
        const std::vector<EarthquakeRecord>& records_for_search_remove_ops,
        const std::vector<EarthquakeRecord>& non_existent_records_for_ops,
        const RestrictionConfig& config = RestrictionConfig{}
    );
    
    // Helper para medir tempo de busca, agora com config
    template<typename Structure>
    double measure_search_time_avg(
        Structure& ds, 
        const std::vector<EarthquakeRecord>& records_to_search,
        const RestrictionConfig& config
    );

    // Helper para info de colisão
    template<typename HashTableType>
    std::string get_hash_collision_info(const HashTableType& ht);

    // Helpers para gerar dados
    std::vector<EarthquakeRecord> generate_non_existent_records(const std::vector<EarthquakeRecord>& existing_records, size_t count);
    std::vector<EarthquakeRecord> prepare_data_for_ops(const std::vector<EarthquakeRecord>& source_data, size_t num_ops);

};

#endif // BENCHMARK_H