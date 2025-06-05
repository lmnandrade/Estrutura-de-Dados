#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "csv_reader.h"
#include "lista_dupla_ligada.h"
#include "avl_tree.h"
#include "hash_table.h"
#include "skip_list.h"
#include "kd_tree.h"
#include "operations_menu.h" // Para KDTreeHelpers

#include <vector>
#include <string>
#include <chrono>
#include <fstream>
#include <map>
#include <iostream>
#include <algorithm>
#include <random>
#include <utility>
#include <type_traits>

// Estrutura para armazenar os resultados de um único teste de benchmark
struct BenchmarkResult {
    std::string data_structure_name;
    std::string operation_type;
    int input_size;
    long long duration_ns;      // Duração da operação em nanossegundos
    long memory_usage_kb;       // Delta de memória em KB (ou uso total, dependendo da medição)
    std::string restriction_scenario;

    BenchmarkResult(std::string ds_name, std::string op, int size, long long dur, long mem, std::string scenario)
        : data_structure_name(std::move(ds_name)), operation_type(std::move(op)),
          input_size(size), duration_ns(dur), memory_usage_kb(mem), restriction_scenario(std::move(scenario)) {}
};

// Classe principal para executar e gerenciar os benchmarks
class BenchmarkRunner {
public:
    BenchmarkRunner(const std::vector<EarthquakeRecord>& all_data, const std::string& scenario_name);

    void run_all_benchmarks();
    void save_results_to_csv(const std::string& filename) const;

private:
    std::vector<EarthquakeRecord> full_dataset;
    std::vector<BenchmarkResult> results;
    std::string current_scenario_name;

    // Métodos template para benchmarking
    template<typename Structure>
    void benchmark_structure(const std::string& structure_name);

    template<typename Structure>
    long long measure_insertion(Structure& ds, const std::vector<EarthquakeRecord>& records_to_insert);

    template<typename Structure>
    long long measure_search_specific(const Structure& ds, const std::vector<EarthquakeRecord>& records_to_search);

    template<typename Structure>
    long long measure_search_random(const Structure& ds, int num_searches);

    template<typename Structure>
    long long measure_removal(Structure& ds, const std::vector<EarthquakeRecord>& records_to_remove);

    // Funções auxiliares (implementadas em benchmark.cpp)
    long get_current_memory_usage_kb();
    std::vector<EarthquakeRecord> get_subset_of_data(int size) const;
    EarthquakeRecord get_random_record_from_subset(const std::vector<EarthquakeRecord>& subset) const;
};

// --- IMPLEMENTAÇÕES DOS MÉTODOS TEMPLATE ---

template<typename Structure>
void BenchmarkRunner::benchmark_structure(const std::string& structure_name) {
    std::cout << "Benchmarking: " << structure_name << " (Cenario: " << current_scenario_name << ")..." << std::endl;
    std::vector<int> sizes_to_test = {100, 500, 1000, 5000, 10000};

    for (int current_size : sizes_to_test) {
        if (static_cast<size_t>(current_size) > full_dataset.size() && !full_dataset.empty()) {
            std::cout << "  Tamanho de teste " << current_size << " excede o dataset original (" << full_dataset.size() << "). Pulando este tamanho." << std::endl;
            continue;
        }
        std::cout << "  Testando com tamanho de entrada: " << current_size << std::endl;

        std::vector<EarthquakeRecord> subset;
        if (full_dataset.empty() && current_size > 0) {
            std::cout << "    AVISO: Dataset original vazio. O subset para o tamanho " << current_size << " estara vazio." << std::endl;
        } else {
            subset = get_subset_of_data(current_size);
        }

        if (subset.empty() && current_size > 0) {
            std::cout << "    AVISO: Subset para o tamanho " << current_size << " esta vazio (apos get_subset_of_data). Pulando operacoes para este tamanho." << std::endl;
            continue;
        }

        Structure ds;

        // 1. Inserção
        long memory_before_op = get_current_memory_usage_kb();
        long long insert_duration = measure_insertion(ds, subset);
        long memory_after_op = get_current_memory_usage_kb();
        long memory_delta = (memory_after_op >= memory_before_op && memory_before_op != 0 && memory_after_op != 0) ? (memory_after_op - memory_before_op) : 0;
        if (memory_before_op == 0 && memory_after_op == 0) memory_delta = 0;
        results.emplace_back(structure_name, "Insertion", current_size, insert_duration, memory_delta, current_scenario_name);
        std::cout << "    Insercao (" << subset.size() << " itens): " << insert_duration << " ns, Memoria Delta: " << memory_delta << " KB" << std::endl;

        if (!subset.empty()) {
            // 2. Busca Específica
            long long search_specific_duration = measure_search_specific(ds, subset);
            results.emplace_back(structure_name, "Search_Specific_All_Subset", current_size, search_specific_duration, 0, current_scenario_name);
            std::cout << "    Busca Especifica (" << subset.size() << " itens): " << search_specific_duration << " ns" << std::endl;

            // 3. Busca Aleatória
            int num_random_searches = std::max(1, static_cast<int>(subset.size() / 10));
            if (num_random_searches > 0) {
                long long search_random_duration = measure_search_random(ds, num_random_searches);
                results.emplace_back(structure_name, "Search_Random_10%", current_size, search_random_duration, 0, current_scenario_name);
                std::cout << "    Busca Aleatoria (" << num_random_searches << " itens): " << search_random_duration << " ns" << std::endl;
            }

            // 4. Remoção
            memory_before_op = get_current_memory_usage_kb();
            long long remove_duration = measure_removal(ds, subset);
            memory_after_op = get_current_memory_usage_kb();
            memory_delta = (memory_before_op >= memory_after_op && memory_before_op != 0 && memory_after_op != 0) ? (memory_before_op - memory_after_op) : 0;
            if (memory_before_op == 0 && memory_after_op == 0) memory_delta = 0;
            results.emplace_back(structure_name, "Removal_All_Subset", current_size, remove_duration, memory_delta, current_scenario_name);
            std::cout << "    Remocao (" << subset.size() << " itens): " << remove_duration << " ns, Memoria Delta: " << memory_delta << " KB" << std::endl;
        }
    }
    std::cout << "Benchmark para " << structure_name << " concluido." << std::endl;
}

template<typename Structure>
long long BenchmarkRunner::measure_insertion(Structure& ds, const std::vector<EarthquakeRecord>& records_to_insert) {
    if (records_to_insert.empty()) return 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto& record : records_to_insert) {
        ds.insert_record(record);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
}

template<typename Structure>
long long BenchmarkRunner::measure_search_specific(const Structure& ds, const std::vector<EarthquakeRecord>& records_to_search) {
    if (records_to_search.empty()) return 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    volatile int found_count = 0;

    for (const auto& record : records_to_search) {
        if constexpr (std::is_same_v<Structure, HashTable> || std::is_same_v<Structure, SkipList>) {
            if (ds.search_record(record.date, record.time, record.city) != nullptr) {
                found_count++;
            }
        } else if constexpr (std::is_same_v<Structure, KDTree>) {
            double lat, lon;
            if (KDTreeHelpers::try_string_to_double(record.latitude, lat) &&
                KDTreeHelpers::try_string_to_double(record.longitude, lon)) {
                if (ds.search_nearest_neighbor(lat, lon) != nullptr) {
                    found_count++;
                }
            }
        } else {
            bool item_found = false;
            std::vector<EarthquakeRecord> all_ds_records;
            if constexpr (std::is_same_v<Structure, AVLTree>) {
                all_ds_records = ds.get_all_records();
            } else if constexpr (std::is_same_v<Structure, DoublyLinkedList>) {
                all_ds_records = ds.get_all_records_vector();
            }
            for (const auto& r_ds : all_ds_records) {
                if (r_ds.date == record.date && r_ds.time == record.time && r_ds.city == record.city && r_ds.country == record.country) {
                    item_found = true;
                    break;
                }
            }
            if (item_found) found_count++;
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
}

template<typename Structure>
long long BenchmarkRunner::measure_search_random(const Structure& ds, int num_searches) {
    if (full_dataset.empty() || num_searches <= 0) return 0;

    std::vector<EarthquakeRecord> records_to_find_randomly;
    int actual_num_searches = std::min(num_searches, static_cast<int>(full_dataset.size()));
    if (actual_num_searches <= 0) return 0;

    std::sample(full_dataset.begin(), full_dataset.end(),
                std::back_inserter(records_to_find_randomly),
                actual_num_searches, std::mt19937{std::random_device{}()});

    auto start_time = std::chrono::high_resolution_clock::now();
    volatile int found_count = 0;
    for (const auto& record_to_find : records_to_find_randomly) {
        if constexpr (std::is_same_v<Structure, HashTable> || std::is_same_v<Structure, SkipList>) {
            if (ds.search_record(record_to_find.date, record_to_find.time, record_to_find.city) != nullptr) {
                found_count++;
            }
        } else if constexpr (std::is_same_v<Structure, KDTree>) {
            double lat, lon;
            if (KDTreeHelpers::try_string_to_double(record_to_find.latitude, lat) &&
                KDTreeHelpers::try_string_to_double(record_to_find.longitude, lon)) {
                if (ds.search_nearest_neighbor(lat, lon) != nullptr) {
                    found_count++;
                }
            }
        } else {
            bool item_found = false;
            std::vector<EarthquakeRecord> all_ds_records;
            if constexpr (std::is_same_v<Structure, AVLTree>) {
                all_ds_records = ds.get_all_records();
            } else if constexpr (std::is_same_v<Structure, DoublyLinkedList>) {
                all_ds_records = ds.get_all_records_vector();
            }
            for (const auto& r_ds : all_ds_records) {
                if (r_ds.date == record_to_find.date && r_ds.time == record_to_find.time && r_ds.city == record_to_find.city && r_ds.country == record_to_find.country) {
                    item_found = true;
                    break;
                }
            }
            if (item_found) found_count++;
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
}

template<typename Structure>
long long BenchmarkRunner::measure_removal(Structure& ds, const std::vector<EarthquakeRecord>& records_to_remove) {
    if (records_to_remove.empty()) return 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto& record : records_to_remove) {
        if constexpr (std::is_same_v<Structure, HashTable>) {
            ds.remove_record(record.date, record.time, record.city);
        } else {
            ds.remove_record(record.date, record.time, record.city, record.country);
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
}

#endif // BENCHMARK_H