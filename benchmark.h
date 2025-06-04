#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "csv_reader.h" // Para EarthquakeRecord
#include "lista_dupla_ligada.h"
#include "avl_tree.h" // <<< DESCOMENTADO
// #include "hash_table.h"
// #include "skip_list.h"
// #include "kd_tree.h"

#include <vector>
#include <string>
#include <chrono>    // Para medição de tempo
#include <fstream>   // Para salvar resultados
#include <map>

// Estrutura para armazenar os resultados de um único teste de benchmark
struct BenchmarkResult {
    std::string data_structure_name;
    std::string operation_type;
    int input_size;
    long long duration_ns;
    long memory_usage_kb;
    std::string restriction_scenario;

    BenchmarkResult(std::string ds_name, std::string op, int size, long long dur, long mem, std::string scenario)
        : data_structure_name(std::move(ds_name)), operation_type(std::move(op)),
          input_size(size), duration_ns(dur), memory_usage_kb(mem), restriction_scenario(std::move(scenario)) {}
};

class BenchmarkRunner {
public:
    BenchmarkRunner(const std::vector<EarthquakeRecord>& all_data, const std::string& scenario_name);

    void run_all_benchmarks();
    void save_results_to_csv(const std::string& filename) const;

private:
    std::vector<EarthquakeRecord> dataset;
    std::vector<BenchmarkResult> results;
    std::string current_scenario_name;

    template<typename Structure>
    void benchmark_structure(Structure& ds, const std::string& structure_name);

    template<typename Structure>
    long long measure_insertion(Structure& ds, const std::vector<EarthquakeRecord>& records_to_insert);

    template<typename Structure>
    long long measure_search_specific(const Structure& ds, const std::vector<EarthquakeRecord>& records_to_search);

    template<typename Structure>
    long long measure_search_random(const Structure& ds, int num_searches);

    template<typename Structure>
    long long measure_removal(Structure& ds, const std::vector<EarthquakeRecord>& records_to_remove);

    long get_current_memory_usage_kb();
    std::vector<EarthquakeRecord> get_subset_of_data(int size) const;
};

#endif // BENCHMARK_H