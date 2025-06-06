#include "benchmark.h"
#include "lista_dupla_ligada.h"
#include "avl_tree.h"
#include "hash_table.h"
#include "kd_tree.h"
#include "skip_list.h"
#include "perfect_hash.h"
// csv_reader.h já é incluído por benchmark.h para EarthquakeRecord

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <numeric>   // Para std::accumulate
#include <algorithm> // Para std::shuffle, std::sort, std::transform, std::min, std::max
#include <random>    // Para std::default_random_engine, std::shuffle, std::mt19937, std::uniform_int_distribution
#include <fstream>   // Para std::ofstream
#include <iomanip>   // Para std::fixed, std::setprecision
#include <map>
#include <sstream>   // Para std::ostringstream (usado em get_hash_collision_info)
#include <thread>    // Para std::this_thread::sleep_for


// --- Namespace KDTreeHelpers (assumindo que está acessível ou definido em um header incluído) ---
// Se não estiver, você precisará da definição de KDTreeHelpers::try_string_to_double aqui.
/*
namespace KDTreeHelpers {
    // ... (definição de try_string_to_double) ...
}
*/


// --- Funções Helper de Medição e Estimativa (coloque-as antes da implementação da classe) ---
template<typename Func>
long long measure_time_ms(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

template<typename Func>
long long measure_time_us(Func&& func) { // microsegundos
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

template<typename NodeStruct>
size_t estimate_node_based_memory(size_t node_count, size_t avg_string_data_per_node = 100) {
    return node_count * (sizeof(NodeStruct) + avg_string_data_per_node);
}

size_t estimate_generic_hash_table_memory(size_t table_vector_size, size_t num_elements, size_t sizeof_hash_node_struct, size_t avg_string_data_per_node = 100) {
    size_t mem = table_vector_size * sizeof(std::list<void*>); 
    mem += num_elements * (sizeof_hash_node_struct + sizeof(void*)*2 + avg_string_data_per_node);
    return mem;
}

size_t estimate_perfect_hash_table_memory(const PerfectHashTable& pht, size_t avg_string_data_per_node = 100) {
    size_t mem = sizeof(pht); 
    mem += pht.get_count() * (sizeof(HashNodePerfect) + avg_string_data_per_node);
    return mem;
}

// --- Implementação BenchmarkRunner ---

BenchmarkRunner::BenchmarkRunner(const std::vector<EarthquakeRecord>& records, const std::string& benchmark_name)
    : all_data_(records), benchmark_name_(benchmark_name), 
      rng_for_benchmarks_(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count())) {
    if (all_data_.empty()) {
        std::cerr << "AVISO (BenchmarkRunner): Dataset vazio fornecido para benchmark." << std::endl;
    }
}

std::vector<EarthquakeRecord> BenchmarkRunner::generate_non_existent_records(const std::vector<EarthquakeRecord>& existing_records, size_t count) {
    std::vector<EarthquakeRecord> non_existent;
    if (existing_records.empty() || count == 0) return non_existent;
    non_existent.reserve(count);
    
    std::mt19937 rng_gen(rng_for_benchmarks_()); // Usa o RNG do runner, mas semeia uma nova instância para esta função

    for (size_t i = 0; i < count; ++i) {
        EarthquakeRecord rec = existing_records[rng_gen() % existing_records.size()];
        rec.city += "_NONEXISTENT_" + std::to_string(rng_gen());
        rec.time = std::to_string(rng_gen() % 100) + ":" + std::to_string(rng_gen() % 100) + ":" + std::to_string(rng_gen() % 100); // Hora inválida
        non_existent.push_back(rec);
    }
    return non_existent;
}

std::vector<EarthquakeRecord> BenchmarkRunner::prepare_data_for_ops(const std::vector<EarthquakeRecord>& source_data, size_t num_ops_target) {
    std::vector<EarthquakeRecord> ops_data;
    if (source_data.empty() || num_ops_target == 0) return ops_data;

    ops_data = source_data; // Copia
    std::shuffle(ops_data.begin(), ops_data.end(), rng_for_benchmarks_);
    
    if (ops_data.size() > num_ops_target) {
        ops_data.resize(num_ops_target);
    }
    return ops_data;
}


void BenchmarkRunner::run_all_benchmarks_and_restricted_tests() {
    if (all_data_.empty()) {
        std::cout << "Nao e possivel executar benchmarks com dataset vazio." << std::endl;
        return;
    }

    std::cout << "\nIniciando Benchmarks: " << benchmark_name_ << std::endl;
    std::cout << "Tamanho Total do Dataset: " << all_data_.size() << " registros." << std::endl;

    // Preparar dados base para operações padrão e escalabilidade
    size_t num_ops_default = std::min((size_t)1000, all_data_.size() / 10);
    if (num_ops_default == 0 && !all_data_.empty()) num_ops_default = std::max((size_t)1, all_data_.size());
    
    std::vector<EarthquakeRecord> search_remove_subset_default = prepare_data_for_ops(all_data_, num_ops_default);
    std::vector<EarthquakeRecord> non_existent_subset_default = generate_non_existent_records(all_data_, num_ops_default);

    // --- 1. Executar Benchmarks Padrão ---
    run_standard_benchmarks(all_data_, search_remove_subset_default, non_existent_subset_default);

    // --- 2. Executar Testes de Escalabilidade ---
    run_scalability_tests();

    // --- 3. Executar Testes Restritos Específicos ---
    std::cout << "\n\n--- Iniciando Testes com Condicoes Restritivas ---" << std::endl;

    // Categoria A: Restrição no tamanho máximo (Memória)
    run_max_size_restriction_tests(std::min((size_t)10000, all_data_.size())); 

    // Categoria B: Simulação de concorrência (Processamento)
    run_concurrency_simulation_tests(std::chrono::milliseconds(2), 50);

    // Categoria C: Simulação de alta latência I/O (Latência)
    run_high_latency_io_tests(std::chrono::milliseconds(5)); // Reduzido para 5ms

    // Categoria D: Amostragem irregular (Disponibilidade de dados)
    run_irregular_arrival_tests(std::chrono::milliseconds(2)); // Reduzido para 2ms

    // Categoria E: Reindexação frequente
    run_frequent_reindex_tests();

    std::cout << "\nTodos Benchmarks (Padrao, Escalabilidade e Restritos) Concluidos." << std::endl;
}

void BenchmarkRunner::run_standard_benchmarks(
    const std::vector<EarthquakeRecord>& data_for_insertion,
    const std::vector<EarthquakeRecord>& records_for_search_remove_ops,
    const std::vector<EarthquakeRecord>& non_existent_records_for_ops) {

    std::cout << "\n--- Benchmarks Padrao ---" << std::endl;
    RestrictionConfig no_restriction_config; 

    std::cout << "Benchmarking DoublyLinkedList (Padrao)..." << std::endl;
    results_["DoublyLinkedList"] = run_benchmark_for_structure<DoublyLinkedList>("DoublyLinkedList", data_for_insertion, records_for_search_remove_ops, non_existent_records_for_ops, no_restriction_config);
    
    std::cout << "Benchmarking AVLTree (Padrao)..." << std::endl;
    results_["AVLTree"] = run_benchmark_for_structure<AVLTree>("AVLTree", data_for_insertion, records_for_search_remove_ops, non_existent_records_for_ops, no_restriction_config);

    std::cout << "Benchmarking HashTable (Padrao)..." << std::endl;
    results_["HashTable"] = run_benchmark_for_structure<HashTable>("HashTable", data_for_insertion, records_for_search_remove_ops, non_existent_records_for_ops, no_restriction_config);
    
    std::cout << "Benchmarking KDTree (Padrao)..." << std::endl;
    results_["KDTree"] = run_benchmark_for_structure<KDTree>("KDTree", data_for_insertion, records_for_search_remove_ops, non_existent_records_for_ops, no_restriction_config);

    std::cout << "Benchmarking SkipList (Padrao)..." << std::endl;
    results_["SkipList"] = run_benchmark_for_structure<SkipList>("SkipList", data_for_insertion, records_for_search_remove_ops, non_existent_records_for_ops, no_restriction_config);
    
    std::cout << "Benchmarking PerfectHashTable (Padrao)..." << std::endl;
    results_["PerfectHashTable"] = run_benchmark_for_structure<PerfectHashTable>("PerfectHashTable", data_for_insertion, records_for_search_remove_ops, non_existent_records_for_ops, no_restriction_config);
}

void BenchmarkRunner::run_scalability_tests() {
    std::cout << "\n--- Testes de Escalabilidade ---" << std::endl;
    std::vector<double> scales = {0.25, 0.50, 0.75, 1.0}; 
    RestrictionConfig no_restriction_cfg; // Padrão para escalabilidade

    for (double scale : scales) {
        size_t subset_size_insert = static_cast<size_t>(all_data_.size() * scale);
        if (subset_size_insert == 0 && !all_data_.empty()) subset_size_insert = 1;
        if (subset_size_insert == 0) continue;

        std::vector<EarthquakeRecord> scaled_data_insert(all_data_.begin(), all_data_.begin() + subset_size_insert);
        std::string scale_suffix = "_Scale" + std::to_string(static_cast<int>(scale * 100)) + "pct";
        
        size_t num_ops_scaled = std::min((size_t)200, subset_size_insert / 10); // Ajustado
        if (num_ops_scaled == 0 && subset_size_insert > 0) num_ops_scaled = 1;
        
        std::vector<EarthquakeRecord> scaled_search_remove_data = prepare_data_for_ops(scaled_data_insert, num_ops_scaled);
        std::vector<EarthquakeRecord> scaled_non_existent = generate_non_existent_records(scaled_data_insert, num_ops_scaled);

        std::cout << "\nBenchmarking Escalabilidade (" << (scale * 100) << "% do dataset, " << subset_size_insert << " registros)..." << std::endl;
        results_["DoublyLinkedList" + scale_suffix] = run_benchmark_for_structure<DoublyLinkedList>("DoublyLinkedList" + scale_suffix, scaled_data_insert, scaled_search_remove_data, scaled_non_existent, no_restriction_cfg);
        results_["AVLTree" + scale_suffix] = run_benchmark_for_structure<AVLTree>("AVLTree" + scale_suffix, scaled_data_insert, scaled_search_remove_data, scaled_non_existent, no_restriction_cfg);
        results_["HashTable" + scale_suffix] = run_benchmark_for_structure<HashTable>("HashTable" + scale_suffix, scaled_data_insert, scaled_search_remove_data, scaled_non_existent, no_restriction_cfg);
        results_["KDTree" + scale_suffix] = run_benchmark_for_structure<KDTree>("KDTree" + scale_suffix, scaled_data_insert, scaled_search_remove_data, scaled_non_existent, no_restriction_cfg);
        results_["SkipList" + scale_suffix] = run_benchmark_for_structure<SkipList>("SkipList" + scale_suffix, scaled_data_insert, scaled_search_remove_data, scaled_non_existent, no_restriction_cfg);
        results_["PerfectHashTable" + scale_suffix] = run_benchmark_for_structure<PerfectHashTable>("PerfectHashTable" + scale_suffix, scaled_data_insert, scaled_search_remove_data, scaled_non_existent, no_restriction_cfg);
    }
}


void BenchmarkRunner::run_max_size_restriction_tests(size_t max_elements_limit) {
    size_t effective_limit = std::min(max_elements_limit, all_data_.size());
    if (effective_limit == 0) return;

    RestrictionConfig cfg(RestrictionMode::MAX_STRUCTURE_SIZE, "_MaxSize" + std::to_string(effective_limit));
    cfg.max_elements = effective_limit;

    std::cout << "\n-- Teste Restrito: Tamanho Maximo da Estrutura (" << cfg.max_elements << " elementos) --" << std::endl;
    
    std::vector<EarthquakeRecord> data_for_test(all_data_.begin(), all_data_.begin() + cfg.max_elements);
    
    size_t num_ops = std::min((size_t)100, data_for_test.size() / 5);
    if (num_ops == 0 && !data_for_test.empty()) num_ops = 1;
        
    std::vector<EarthquakeRecord> ops_subset = prepare_data_for_ops(data_for_test, num_ops);
    std::vector<EarthquakeRecord> non_existent_ops = generate_non_existent_records(data_for_test, num_ops);

    results_["AVLTree" + cfg.name_suffix] = run_benchmark_for_structure<AVLTree>("AVLTree" + cfg.name_suffix, data_for_test, ops_subset, non_existent_ops, cfg);
    results_["HashTable" + cfg.name_suffix] = run_benchmark_for_structure<HashTable>("HashTable" + cfg.name_suffix, data_for_test, ops_subset, non_existent_ops, cfg);
    results_["DoublyLinkedList" + cfg.name_suffix] = run_benchmark_for_structure<DoublyLinkedList>("DoublyLinkedList" + cfg.name_suffix, data_for_test, ops_subset, non_existent_ops, cfg);
}

void BenchmarkRunner::run_concurrency_simulation_tests(std::chrono::milliseconds yield_time, int ops_per_yield) {
    RestrictionConfig cfg(RestrictionMode::SIMULATE_CONCURRENCY, "_ConcurrencySim");
    cfg.concurrency_yield_time = yield_time;
    cfg.concurrency_ops_per_yield = ops_per_yield;

    std::cout << "\n-- Teste Restrito: Simulacao de Concorrencia (pausa " 
              << cfg.concurrency_yield_time.count() << "ms a cada " 
              << cfg.concurrency_ops_per_yield << " ops) --" << std::endl;

    size_t num_ops = std::min((size_t)1000, all_data_.size() / 10);
    if (num_ops == 0 && !all_data_.empty()) num_ops = 1;
    std::vector<EarthquakeRecord> search_remove_subset = prepare_data_for_ops(all_data_, num_ops);
    std::vector<EarthquakeRecord> non_existent_subset = generate_non_existent_records(all_data_, num_ops);

    results_["DoublyLinkedList" + cfg.name_suffix] = run_benchmark_for_structure<DoublyLinkedList>("DoublyLinkedList" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
    results_["AVLTree" + cfg.name_suffix] = run_benchmark_for_structure<AVLTree>("AVLTree" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
    results_["SkipList" + cfg.name_suffix] = run_benchmark_for_structure<SkipList>("SkipList" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
}

void BenchmarkRunner::run_high_latency_io_tests(std::chrono::milliseconds max_io_latency) {
    RestrictionConfig cfg(RestrictionMode::SIMULATE_HIGH_LATENCY_IO, "_HighLatencyIO");
    cfg.io_max_latency_ms = max_io_latency;
    cfg.rng_engine = &rng_for_benchmarks_; 

    std::cout << "\n-- Teste Restrito: Simulacao de Alta Latencia I/O (delay ate " 
              << cfg.io_max_latency_ms.count() << "ms/op) --" << std::endl;
    
    size_t num_ops = std::min((size_t)200, all_data_.size() / 10); // Menos ops para testes com delay
     if (num_ops == 0 && !all_data_.empty()) num_ops = 1;
    std::vector<EarthquakeRecord> search_remove_subset = prepare_data_for_ops(all_data_, num_ops);
    std::vector<EarthquakeRecord> non_existent_subset = generate_non_existent_records(all_data_, num_ops);

    results_["AVLTree" + cfg.name_suffix] = run_benchmark_for_structure<AVLTree>("AVLTree" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
    results_["HashTable" + cfg.name_suffix] = run_benchmark_for_structure<HashTable>("HashTable" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
    results_["PerfectHashTable" + cfg.name_suffix] = run_benchmark_for_structure<PerfectHashTable>("PerfectHashTable" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
}

void BenchmarkRunner::run_irregular_arrival_tests(std::chrono::milliseconds max_arrival_interval) {
    RestrictionConfig cfg(RestrictionMode::IRREGULAR_DATA_ARRIVAL, "_IrregularArrival");
    cfg.arrival_max_interval_ms = max_arrival_interval;
    cfg.rng_engine = &rng_for_benchmarks_;

    std::cout << "\n-- Teste Restrito: Amostragem Irregular na Insercao (intervalo ate " 
              << cfg.arrival_max_interval_ms.count() << "ms) --" << std::endl;
    
    size_t num_ops = std::min((size_t)200, all_data_.size() / 10);
     if (num_ops == 0 && !all_data_.empty()) num_ops = 1;
    std::vector<EarthquakeRecord> search_remove_subset = prepare_data_for_ops(all_data_, num_ops);
    std::vector<EarthquakeRecord> non_existent_subset = generate_non_existent_records(all_data_, num_ops);

    results_["KDTree" + cfg.name_suffix] = run_benchmark_for_structure<KDTree>("KDTree" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
    results_["HashTable" + cfg.name_suffix] = run_benchmark_for_structure<HashTable>("HashTable" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
    results_["SkipList" + cfg.name_suffix] = run_benchmark_for_structure<SkipList>("SkipList" + cfg.name_suffix, all_data_, search_remove_subset, non_existent_subset, cfg);
}

void BenchmarkRunner::run_frequent_reindex_tests() {
    if (all_data_.size() < 50) { 
        std::cout << "\n-- Teste Restrito: Reindexacao Frequente (PHT) - Ignorado (dataset pequeno demais) --" << std::endl;
        return;
    }

    std::cout << "\n-- Teste Restrito: Reindexacao Frequente (PerfectHashTable Insert Post-Build) --" << std::endl;
    
    size_t initial_build_percentage = 80; 
    size_t initial_build_size = (all_data_.size() * initial_build_percentage) / 100;
    if(initial_build_size == 0 && !all_data_.empty()) initial_build_size = std::min((size_t)1, all_data_.size() -1);
    if(initial_build_size == all_data_.size()) initial_build_size--; // garantir que há algo para inserir depois
    if(initial_build_size == 0) return; // Ainda não há dados suficientes
    
    std::vector<EarthquakeRecord> initial_data(all_data_.begin(), all_data_.begin() + initial_build_size);
    std::vector<EarthquakeRecord> post_build_insert_data(all_data_.begin() + initial_build_size, all_data_.end());

    if (post_build_insert_data.empty()){
         std::cout << "  Nao ha dados suficientes para insercao pos-build. Ignorando." << std::endl;
         return;
    }

    PerfectHashTable pht_reindex_test(all_data_.size());
    BenchmarkMetrics pht_metrics; 
    RestrictionConfig no_restriction_for_ops; // as operações de busca/rem depois serão padrão

    std::cout << "  Construindo PHT com " << initial_data.size() << " registros..." << std::endl;
    pht_metrics.insertion_time_ms = measure_time_ms([&]() {
        pht_reindex_test.build_table(initial_data);
    });
    pht_metrics.actual_elements_processed_insertion = pht_reindex_test.get_count();

    std::cout << "  Inserindo " << post_build_insert_data.size() << " registros adicionais..." << std::endl;
    long long post_build_insertion_time_us = 0;
    post_build_insertion_time_us = measure_time_us([&]() {
        for (const auto& rec : post_build_insert_data) {
            pht_reindex_test.insert_record(rec);
        }
    });
    pht_metrics.insertion_time_ms += (post_build_insertion_time_us + 500) / 1000; // Adiciona ao tempo total (com arredondamento)
    pht_metrics.actual_elements_processed_insertion = pht_reindex_test.get_count();
    
    if (!post_build_insert_data.empty()) {
        double avg_us_per_post_build_elem = static_cast<double>(post_build_insertion_time_us) / post_build_insert_data.size();
        std::cout << "    Tempo medio para insercoes pos-build: " 
                  << std::fixed << std::setprecision(2) << avg_us_per_post_build_elem
                  << " us/elem" << std::endl;
    }
    
    if (pht_metrics.actual_elements_processed_insertion > 0) {
        pht_metrics.insertion_time_us_avg_element = (static_cast<double>(pht_metrics.insertion_time_ms) * 1000.0) / pht_metrics.actual_elements_processed_insertion;
    }
    pht_metrics.collision_rate_info = get_hash_collision_info(pht_reindex_test);
    
    // Opcional: realizar buscas/remoções na PHT após inserções adicionais
    // Linha 339 (aproximadamente) no benchmark.cpp
    size_t num_ops_pht = std::min((size_t)50, static_cast<size_t>(pht_reindex_test.get_count() / 5));
    if (num_ops_pht == 0 && pht_reindex_test.get_count() > 0) num_ops_pht = 1;

    std::vector<EarthquakeRecord> ops_for_pht_reindex = prepare_data_for_ops(all_data_, num_ops_pht); // Usa all_data_ para pegar algumas que foram inseridas
    std::vector<EarthquakeRecord> nonexist_ops_pht_reindex = generate_non_existent_records(all_data_, num_ops_pht);
    
    if(!ops_for_pht_reindex.empty()){
        pht_metrics.search_existing_time_us_avg = measure_search_time_avg(pht_reindex_test, ops_for_pht_reindex, no_restriction_for_ops);
        pht_metrics.actual_elements_processed_ops = ops_for_pht_reindex.size();
        // ... (poderia adicionar remoção e outras métricas aqui se desejado)
    }

    results_["PerfectHashTable_PostBuildInsert"] = pht_metrics;
}


template<typename Structure>
BenchmarkMetrics BenchmarkRunner::run_benchmark_for_structure(
    const std::string& structure_id_prefix,
    const std::vector<EarthquakeRecord>& data_for_insertion_source,
    const std::vector<EarthquakeRecord>& records_for_search_remove_ops_source,
    const std::vector<EarthquakeRecord>& non_existent_records_for_ops_source,
    const RestrictionConfig& config) {

    BenchmarkMetrics metrics;
    Structure ds; 
    metrics.actual_elements_processed_insertion = 0;
    metrics.actual_elements_processed_ops = 0;

    if constexpr (std::is_constructible_v<Structure, size_t>) {
        ds = Structure(config.mode == RestrictionMode::MAX_STRUCTURE_SIZE && config.max_elements > 0 ? 
                       config.max_elements : data_for_insertion_source.size());
    }
    
    std::vector<EarthquakeRecord> effective_data_for_insertion = data_for_insertion_source;
    if (config.mode == RestrictionMode::MAX_STRUCTURE_SIZE && config.max_elements > 0 && config.max_elements < data_for_insertion_source.size()) {
        effective_data_for_insertion.assign(data_for_insertion_source.begin(), data_for_insertion_source.begin() + config.max_elements);
    }
    metrics.actual_elements_processed_insertion = effective_data_for_insertion.size();


    // --- 1. Tempo de Inserção ---
    std::cout << "  Testando Insercao (" << structure_id_prefix << config.name_suffix <<")...";
    long long current_op_count_concurrency = 0;

    if constexpr (std::is_same_v<Structure, PerfectHashTable>) {
        if (config.mode == RestrictionMode::SIMULATE_CONCURRENCY || config.mode == RestrictionMode::IRREGULAR_DATA_ARRIVAL || config.mode == RestrictionMode::SIMULATE_HIGH_LATENCY_IO) {
            std::cout << " (Restricoes de tempo real nao aplicaveis ao build da PHT, usando insercao padrao)...";
        }
        metrics.insertion_time_ms = measure_time_ms([&]() {
            ds.build_table(effective_data_for_insertion); 
        });
    } else {
        metrics.insertion_time_ms = measure_time_ms([&]() {
            for (const auto& record : effective_data_for_insertion) {
                if (config.mode == RestrictionMode::IRREGULAR_DATA_ARRIVAL && config.arrival_max_interval_ms.count() > 0 && config.rng_engine) {
                    std::uniform_int_distribution<> distrib(1, static_cast<int>(config.arrival_max_interval_ms.count()));
                    std::this_thread::sleep_for(std::chrono::milliseconds(distrib(*(config.rng_engine))));
                }
                if (config.mode == RestrictionMode::SIMULATE_HIGH_LATENCY_IO && config.io_max_latency_ms.count() > 0 && config.rng_engine) {
                    std::uniform_int_distribution<> distrib(0, static_cast<int>(config.io_max_latency_ms.count()));
                    std::this_thread::sleep_for(std::chrono::milliseconds(distrib(*(config.rng_engine))));
                }
                
                ds.insert_record(record);

                if (config.mode == RestrictionMode::SIMULATE_CONCURRENCY) {
                    current_op_count_concurrency++;
                    if (current_op_count_concurrency % config.concurrency_ops_per_yield == 0) {
                        std::this_thread::sleep_for(config.concurrency_yield_time);
                    }
                }
            }
        });
    }
    if (metrics.actual_elements_processed_insertion > 0) {
        metrics.insertion_time_us_avg_element = (static_cast<double>(metrics.insertion_time_ms) * 1000.0) / metrics.actual_elements_processed_insertion;
    }
    std::cout << " Feito. (" << metrics.insertion_time_ms << " ms)" << std::endl;
    
    std::vector<EarthquakeRecord> effective_records_for_ops = records_for_search_remove_ops_source;
    std::vector<EarthquakeRecord> effective_non_existent_ops = non_existent_records_for_ops_source;

    if (config.mode == RestrictionMode::MAX_STRUCTURE_SIZE && config.max_elements > 0 && !records_for_search_remove_ops_source.empty()) {
        // Garantir que ops sejam feitas em dados inseridos
        effective_records_for_ops.clear();
        std::vector<EarthquakeRecord> temp_shuffled_inserted = effective_data_for_insertion;
        std::shuffle(temp_shuffled_inserted.begin(), temp_shuffled_inserted.end(), rng_for_benchmarks_);
        
        size_t ops_count_target = std::min(records_for_search_remove_ops_source.size(), temp_shuffled_inserted.size());
        if(ops_count_target > 0) {
            effective_records_for_ops.assign(temp_shuffled_inserted.begin(), temp_shuffled_inserted.begin() + ops_count_target);
        }
        // Ajustar non_existent também para ter um tamanho comparável
        if (effective_non_existent_ops.size() > ops_count_target) effective_non_existent_ops.resize(ops_count_target);
    }
     metrics.actual_elements_processed_ops = effective_records_for_ops.size();

    if (metrics.actual_elements_processed_ops > 0){
        std::cout << "  Testando Busca (Existente) (" << structure_id_prefix << config.name_suffix <<")...";
        metrics.search_existing_time_us_avg = measure_search_time_avg(ds, effective_records_for_ops, config);
        std::cout << " Feito." << std::endl;

        if(!effective_non_existent_ops.empty()){
             std::cout << "  Testando Busca (Inexistente) (" << structure_id_prefix << config.name_suffix <<")...";
            metrics.search_non_existent_time_us_avg = measure_search_time_avg(ds, effective_non_existent_ops, config);
            std::cout << " Feito." << std::endl;
        }
        metrics.avg_access_time_us = metrics.search_existing_time_us_avg;

        std::cout << "  Testando Remocao (" << structure_id_prefix << config.name_suffix <<")...";
        current_op_count_concurrency = 0; // Reset
        long long total_removal_us = measure_time_us([&]() {
            for (const auto& rec_to_remove : effective_records_for_ops) {
                 if (config.mode == RestrictionMode::SIMULATE_HIGH_LATENCY_IO && config.io_max_latency_ms.count() > 0 && config.rng_engine) {
                    std::uniform_int_distribution<> distrib(0, static_cast<int>(config.io_max_latency_ms.count()));
                    std::this_thread::sleep_for(std::chrono::milliseconds(distrib(*(config.rng_engine))));
                }

                if constexpr (std::is_same_v<Structure, AVLTree> || std::is_same_v<Structure, KDTree> || std::is_same_v<Structure, DoublyLinkedList> || std::is_same_v<Structure, SkipList>) {
                     ds.remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city, rec_to_remove.country);
                } else {
                    ds.remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city);
                }

                if (config.mode == RestrictionMode::SIMULATE_CONCURRENCY) {
                    current_op_count_concurrency++;
                    if (current_op_count_concurrency % config.concurrency_ops_per_yield == 0) {
                        std::this_thread::sleep_for(config.concurrency_yield_time);
                    }
                }
            }
        });
        if (metrics.actual_elements_processed_ops > 0) {
            metrics.removal_time_us_avg = static_cast<double>(total_removal_us) / metrics.actual_elements_processed_ops;
        }
        std::cout << " Feito." << std::endl;
    }
    
    size_t elements_for_mem_estimation = ds.get_count(); 
    if (elements_for_mem_estimation == 0 && config.mode == RestrictionMode::MAX_STRUCTURE_SIZE) elements_for_mem_estimation = config.max_elements;
    else if (elements_for_mem_estimation == 0) elements_for_mem_estimation = metrics.actual_elements_processed_insertion;
    
    if constexpr (std::is_same_v<Structure, DoublyLinkedList>) metrics.memory_usage_bytes = estimate_node_based_memory<Node>(elements_for_mem_estimation);
    else if constexpr (std::is_same_v<Structure, AVLTree>) metrics.memory_usage_bytes = estimate_node_based_memory<AVLNode>(elements_for_mem_estimation);
    else if constexpr (std::is_same_v<Structure, HashTable>) metrics.memory_usage_bytes = estimate_generic_hash_table_memory(elements_for_mem_estimation * 1.5, elements_for_mem_estimation, sizeof(HashNode)); // Aprox table size
    else if constexpr (std::is_same_v<Structure, KDTree>) metrics.memory_usage_bytes = estimate_node_based_memory<KDNode>(elements_for_mem_estimation);
    else if constexpr (std::is_same_v<Structure, SkipList>) metrics.memory_usage_bytes = estimate_node_based_memory<SkipListNode>(elements_for_mem_estimation); // Simplificado, SkipList é mais complexo
    else if constexpr (std::is_same_v<Structure, PerfectHashTable>) metrics.memory_usage_bytes = estimate_perfect_hash_table_memory(ds);
    else metrics.memory_usage_bytes = elements_for_mem_estimation * (sizeof(EarthquakeRecord) + 100); 


    if constexpr (std::is_same_v<Structure, HashTable> || std::is_same_v<Structure, PerfectHashTable>) {
        metrics.collision_rate_info = get_hash_collision_info(ds);
    }

     if (metrics.actual_elements_processed_ops > 0){
        size_t num_mixed_ops = std::min((size_t)50, metrics.actual_elements_processed_ops / 2); 
        if (num_mixed_ops == 0 && metrics.actual_elements_processed_ops > 0) num_mixed_ops = 1;
        
        if (num_mixed_ops > 0) {
            Structure temp_ds_for_latency; 
             if constexpr (std::is_constructible_v<Structure, size_t>) {
                temp_ds_for_latency = Structure(num_mixed_ops * 2);
            }

            std::vector<EarthquakeRecord> mixed_ops_data_insert = effective_records_for_ops; // Usa os dados já filtrados/ajustados
            if (mixed_ops_data_insert.size() > num_mixed_ops) mixed_ops_data_insert.resize(num_mixed_ops);
            std::vector<EarthquakeRecord> mixed_ops_data_search_remove = mixed_ops_data_insert; 

            current_op_count_concurrency = 0;
            long long total_mixed_ops_us = measure_time_us([&]() {
                if constexpr (std::is_same_v<Structure, PerfectHashTable>) {
                    temp_ds_for_latency.build_table(mixed_ops_data_insert);
                } else {
                    for (const auto& rec : mixed_ops_data_insert) {
                        if (config.mode == RestrictionMode::SIMULATE_HIGH_LATENCY_IO && config.io_max_latency_ms.count() > 0 && config.rng_engine) { std::this_thread::sleep_for(std::chrono::milliseconds(std::uniform_int_distribution<>(0, static_cast<int>(config.io_max_latency_ms.count()))(*(config.rng_engine)))); }
                        temp_ds_for_latency.insert_record(rec);
                        if (config.mode == RestrictionMode::SIMULATE_CONCURRENCY) { current_op_count_concurrency++; if (current_op_count_concurrency % config.concurrency_ops_per_yield == 0) { std::this_thread::sleep_for(config.concurrency_yield_time); } }
                    }
                }
                for (const auto& rec : mixed_ops_data_search_remove) {
                    if (config.mode == RestrictionMode::SIMULATE_HIGH_LATENCY_IO && config.io_max_latency_ms.count() > 0 && config.rng_engine) { std::this_thread::sleep_for(std::chrono::milliseconds(std::uniform_int_distribution<>(0, static_cast<int>(config.io_max_latency_ms.count()))(*(config.rng_engine)))); }
                    if constexpr (std::is_same_v<Structure, DoublyLinkedList> || std::is_same_v<Structure, KDTree>) { volatile auto f = temp_ds_for_latency.search_by_city(rec.city); }
                    else if constexpr (std::is_same_v<Structure, AVLTree>) { volatile auto f = temp_ds_for_latency.search_record_by_composite_key(rec.date, rec.time, rec.city, rec.country); }
                    else { volatile auto f = temp_ds_for_latency.search_record(rec.date, rec.time, rec.city); }
                    if (config.mode == RestrictionMode::SIMULATE_CONCURRENCY) { current_op_count_concurrency++; if (current_op_count_concurrency % config.concurrency_ops_per_yield == 0) { std::this_thread::sleep_for(config.concurrency_yield_time); } }
                }
                for (const auto& rec : mixed_ops_data_search_remove) {
                     if (config.mode == RestrictionMode::SIMULATE_HIGH_LATENCY_IO && config.io_max_latency_ms.count() > 0 && config.rng_engine) { std::this_thread::sleep_for(std::chrono::milliseconds(std::uniform_int_distribution<>(0, static_cast<int>(config.io_max_latency_ms.count()))(*(config.rng_engine)))); }
                    if constexpr (std::is_same_v<Structure, AVLTree> || std::is_same_v<Structure, KDTree> || std::is_same_v<Structure, DoublyLinkedList> || std::is_same_v<Structure, SkipList>) { temp_ds_for_latency.remove_record(rec.date, rec.time, rec.city, rec.country); }
                    else { temp_ds_for_latency.remove_record(rec.date, rec.time, rec.city); }
                     if (config.mode == RestrictionMode::SIMULATE_CONCURRENCY) { current_op_count_concurrency++; if (current_op_count_concurrency % config.concurrency_ops_per_yield == 0) { std::this_thread::sleep_for(config.concurrency_yield_time); } }
                }
            });
            size_t num_actual_mixed_ops_count = (std::is_same_v<Structure, PerfectHashTable>) ? (1 + 2 * mixed_ops_data_search_remove.size()) : (mixed_ops_data_insert.size() + 2 * mixed_ops_data_search_remove.size());
            if (num_actual_mixed_ops_count > 0) {
                 metrics.avg_latency_us_mixed_ops = static_cast<double>(total_mixed_ops_us) / num_actual_mixed_ops_count;
            }
        }
    }

    std::cout << "    Resumo para " << structure_id_prefix << config.name_suffix << ":" << std::endl;
    std::cout << "      Elementos Inseridos: " << metrics.actual_elements_processed_insertion << ", Elementos para Ops: " << metrics.actual_elements_processed_ops << std::endl;
    std::cout << "      Insercao Total: " << metrics.insertion_time_ms << " ms"
              << " (Media: " << std::fixed << std::setprecision(2) << metrics.insertion_time_us_avg_element << " us/elem)" << std::endl;
    if(metrics.actual_elements_processed_ops > 0){
        std::cout << "      Busca (Existente): " << std::fixed << std::setprecision(2) << metrics.search_existing_time_us_avg << " us/op" << std::endl;
        if(!effective_non_existent_ops.empty()) std::cout << "      Busca (Inexistente): " << std::fixed << std::setprecision(2) << metrics.search_non_existent_time_us_avg << " us/op" << std::endl;
        std::cout << "      Remocao: " << std::fixed << std::setprecision(2) << metrics.removal_time_us_avg << " us/op" << std::endl;
        std::cout << "      Latencia Media (Ops Mistas): " << std::fixed << std::setprecision(2) << metrics.avg_latency_us_mixed_ops << " us/op" << std::endl;
    }
    std::cout << "      Memoria Estimada: " << metrics.memory_usage_bytes / (1024) << " KB" << std::endl;
     if (!metrics.collision_rate_info.empty() && metrics.collision_rate_info.find("N/A") == std::string::npos) {
        std::cout << "      Colisoes Info: " << metrics.collision_rate_info << std::endl;
    }

    return metrics;
}

template<typename Structure>
double BenchmarkRunner::measure_search_time_avg(
    Structure& ds, 
    const std::vector<EarthquakeRecord>& records_to_search,
    const RestrictionConfig& config) {
    if (records_to_search.empty()) return 0.0;
    long long current_op_count_concurrency = 0;

    long long total_us = measure_time_us([&]() {
        for (const auto& rec : records_to_search) {
            if (config.mode == RestrictionMode::SIMULATE_HIGH_LATENCY_IO && config.io_max_latency_ms.count() > 0 && config.rng_engine) {
                std::uniform_int_distribution<> distrib(0, static_cast<int>(config.io_max_latency_ms.count()));
                std::this_thread::sleep_for(std::chrono::milliseconds(distrib(*(config.rng_engine))));
            }

            if constexpr (std::is_same_v<Structure, DoublyLinkedList> || std::is_same_v<Structure, KDTree>) {
                 volatile auto found = ds.search_by_city(rec.city); 
            } else if constexpr (std::is_same_v<Structure, AVLTree>) {
                 volatile const EarthquakeRecord* found = ds.search_record_by_composite_key(rec.date, rec.time, rec.city, rec.country);
            } else { 
                volatile const EarthquakeRecord* found = ds.search_record(rec.date, rec.time, rec.city);
            }

            if (config.mode == RestrictionMode::SIMULATE_CONCURRENCY) {
                current_op_count_concurrency++;
                if (current_op_count_concurrency % config.concurrency_ops_per_yield == 0) {
                    std::this_thread::sleep_for(config.concurrency_yield_time);
                }
            }
        }
    });
    return static_cast<double>(total_us) / records_to_search.size();
}

template<typename HashTableType>
std::string BenchmarkRunner::get_hash_collision_info(const HashTableType& ht) {
    if constexpr (std::is_same_v<HashTableType, PerfectHashTable>) {
        // A PHT deveria ter 0 colisões secundárias por design.
        // Poderíamos adicionar lógica para verificar a distribuição primária se a PHT expuser isso.
        return "PHT: Secundarias 0 (design); Primarias: (Nao verificado)";
    } else if constexpr (std::is_same_v<HashTableType, HashTable>) {
        // Se HashTable tivesse um método `get_collision_stats()`
        // ht.get_collision_stats().max_chain_length, etc.
        // Por enquanto:
        return "N/A (Implementar get_collision_stats em HashTable)";
    }
    return "N/A";
}

void BenchmarkRunner::save_results_to_csv(const std::string& filepath) const {
    std::ofstream outfile(filepath);
    if (!outfile.is_open()) {
        std::cerr << "ERRO: Nao foi possivel abrir o arquivo para salvar os resultados do benchmark: " << filepath << std::endl;
        return;
    }

    outfile << "Estrutura,Metrica,Valor,Unidade,ElementosInseridos,ElementosOps\n";

    for (const auto& pair_structure_metrics : results_) {
        const std::string& struct_name = pair_structure_metrics.first;
        const BenchmarkMetrics& metrics = pair_structure_metrics.second;

        auto write_line = [&](const std::string& metric_name, const auto& value, const std::string& unit){
            outfile << struct_name << "," << metric_name << ",";
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(value)>>) {
                 outfile << std::fixed << std::setprecision(3) << value;
            } else {
                 std::string val_str = value;
                 std::replace(val_str.begin(), val_str.end(), ',', ';'); // Evita que vírgula no valor quebre CSV
                 outfile << """ << val_str << """;
            }
            outfile << "," << unit 
                    << "," << metrics.actual_elements_processed_insertion
                    << "," << metrics.actual_elements_processed_ops
                    << "\n";
        };
        
        write_line("TempoInsercaoTotal", metrics.insertion_time_ms, "ms");
        write_line("TempoInsercaoMedioPorElemento", metrics.insertion_time_us_avg_element, "us");
        if (metrics.actual_elements_processed_ops > 0) { // Apenas se houveram operações
            write_line("TempoBuscaExistenteMedio", metrics.search_existing_time_us_avg, "us");
            write_line("TempoBuscaInexistenteMedio", metrics.search_non_existent_time_us_avg, "us");
            write_line("TempoRemocaoMedio", metrics.removal_time_us_avg, "us");
            write_line("TempoMedioAcesso", metrics.avg_access_time_us, "us");
            write_line("LatenciaMediaOpsMistas", metrics.avg_latency_us_mixed_ops, "us");
        }
        write_line("UsoMemoriaEstimado", metrics.memory_usage_bytes, "bytes");
        write_line("TaxaColisoesInfo", metrics.collision_rate_info, "texto");
    }

    outfile.close();
    std::cout << "Resultados do benchmark salvos em: " << filepath << std::endl;
}