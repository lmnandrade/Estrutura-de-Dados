#include "benchmark.h"
#include "avl_tree.h" // <<< INCLUÍDO (se já não estiver indiretamente pelo benchmark.h)
#include <iostream>
#include <algorithm>
#include <random>
#include <iomanip>
#include <type_traits> // Para std::is_same_v

#ifdef __linux__
#include <sys/resource.h>
#endif

BenchmarkRunner::BenchmarkRunner(const std::vector<EarthquakeRecord>& all_data, const std::string& scenario_name)
    : dataset(all_data), current_scenario_name(scenario_name) {
    if (dataset.empty()) {
        std::cerr << "AVISO (Benchmark): Dataset vazio fornecido para benchmarking." << std::endl;
    }
}

std::vector<EarthquakeRecord> BenchmarkRunner::get_subset_of_data(int size) const {
    if (size <= 0 || dataset.empty()) return {};
    if (static_cast<size_t>(size) >= dataset.size()) return dataset;

    std::vector<EarthquakeRecord> subset;
    std::sample(dataset.begin(), dataset.end(), std::back_inserter(subset),
                size, std::mt19937{std::random_device{}()});
    return subset;
}

long BenchmarkRunner::get_current_memory_usage_kb() {
#ifdef __linux__
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss;
    } else {
        // std::cerr << "Erro ao obter uso de memoria com getrusage." << std::endl;
        return 0;
    }
#else
    return 0;
#endif
}

template<typename Structure>
long long BenchmarkRunner::measure_insertion(Structure& ds, const std::vector<EarthquakeRecord>& records_to_insert) {
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
    long long total_duration_ns = 0;
    volatile bool found_flag = false; // Para evitar que o compilador otimize a busca

    auto start_time_total = std::chrono::high_resolution_clock::now();
    for (const auto& record_criteria : records_to_search) {
        if constexpr (std::is_same_v<Structure, DoublyLinkedList>) {
             // Para DLL, uma busca "específica" pode ser por um de seus métodos de busca
             // Vamos usar search_by_city como exemplo, mas o ideal seria buscar por todos os campos
             // se houvesse um método para isso, ou adaptar o que significa "específico".
             auto result = ds.search_by_city(record_criteria.city);
             if(!result.empty()) found_flag = true;
        } else if constexpr (std::is_same_v<Structure, AVLTree>) {
            // Para AVLTree, a busca otimizada é pela chave composta.
            const EarthquakeRecord* found = ds.search_record_by_composite_key(
                record_criteria.date, record_criteria.time, record_criteria.city, record_criteria.country
            );
            if (found) found_flag = true;
        } else {
            // Lógica genérica ou erro se a estrutura não for conhecida
            // Para este exemplo, vamos assumir que se não for DLL ou AVL, não fazemos nada específico
            // ou você pode adicionar mais `else if constexpr` para outras estruturas.
            // Ex: auto result = ds.find_record(record_criteria); // Se existir um método genérico
        }
    }
    auto end_time_total = std::chrono::high_resolution_clock::now();
    total_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_total - start_time_total).count();
    (void)found_flag; // Usa a flag para evitar otimização

    return total_duration_ns / (records_to_search.empty() ? 1 : records_to_search.size());
}

namespace {
    // Helper to detect is_empty()
    template<typename T>
    auto has_is_empty(int) -> decltype(std::declval<const T&>().is_empty(), std::true_type{});
    template<typename T>
    std::false_type has_is_empty(...);

    // Helper to detect get_count()
    template<typename T>
    auto has_get_count(int) -> decltype(std::declval<const T&>().get_count(), std::true_type{});
    template<typename T>
    std::false_type has_get_count(...);
}

template<typename Structure>
long long BenchmarkRunner::measure_search_random(const Structure& ds, int num_searches) {
    if (num_searches <= 0) return 0;

    // SFINAE-based check for is_empty() or get_count()
    if constexpr (decltype(has_is_empty<Structure>(0))::value) {
        if (ds.is_empty()) return 0;
    } else if constexpr (decltype(has_get_count<Structure>(0))::value) {
        if (ds.get_count() == 0) return 0;
    }
    // Se não houver como checar, prosseguimos com cautela.

    std::vector<EarthquakeRecord> records_to_search_randomly;
    // Pega amostras do `dataset` que *deveriam* estar na estrutura se foi populada com ele.
    if (!dataset.empty()) {
        int effective_num_searches = std::min(num_searches, static_cast<int>(dataset.size()));
        if (effective_num_searches > 0) {
            std::sample(dataset.begin(), dataset.end(), std::back_inserter(records_to_search_randomly),
                        effective_num_searches, std::mt19937{std::random_device{}()});
        }
    }

    if (records_to_search_randomly.empty()) {
        // std::cout << "    Aviso (Search Random): Nao foi possivel selecionar registros para busca aleatoria." << std::endl;
        return 0;
    }

    return measure_search_specific(ds, records_to_search_randomly);
}

template<typename Structure>
long long BenchmarkRunner::measure_removal(Structure& ds, const std::vector<EarthquakeRecord>& records_to_remove) {
    auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto& record : records_to_remove) {
        ds.remove_record(record.date, record.time, record.city, record.country);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
}

template<typename Structure>
void BenchmarkRunner::benchmark_structure(Structure& ds_template_arg, const std::string& structure_name) {
    // ds_template_arg não é usada diretamente, pois cada teste de tamanho cria sua instância
    (void)ds_template_arg; // Evita warning de não utilizada

    std::cout << "\n--- Benchmarking: " << structure_name << " (Cenario: " << current_scenario_name << ") ---" << std::endl;

    std::vector<int> input_sizes;
    if (dataset.size() == 0) { // Adicionado check para dataset vazio
        std::cout << "  Dataset vazio, nenhum benchmark sera executado para " << structure_name << std::endl;
        return;
    }
    if (dataset.size() < 100) input_sizes = { (int)dataset.size() };
    else if (dataset.size() < 1000) input_sizes = {10, 50, (int)dataset.size() / 2, (int)dataset.size()};
    else if (dataset.size() < 10000) input_sizes = {100, 500, 1000, (int)dataset.size() / 2, (int)dataset.size()};
    else input_sizes = {1000, 5000, 10000, (int)dataset.size() / 2, (int)dataset.size()};

    std::sort(input_sizes.begin(), input_sizes.end());
    input_sizes.erase(std::unique(input_sizes.begin(), input_sizes.end()), input_sizes.end());
    input_sizes.erase(std::remove_if(input_sizes.begin(), input_sizes.end(),
                                   [&](int s){ return s <=0 || static_cast<size_t>(s) > dataset.size(); }),
                      input_sizes.end());
    if (input_sizes.empty() && !dataset.empty()) {
        input_sizes.push_back(dataset.size());
    } else if (input_sizes.empty() && dataset.empty()){
        // Já coberto pelo check no início da função
        return;
    }


    for (int current_size : input_sizes) {
        std::cout << "  Tamanho da Entrada: " << current_size << std::endl;

        std::vector<EarthquakeRecord> data_subset = get_subset_of_data(current_size);
        if (data_subset.empty()) {
            std::cout << "    Aviso: Subconjunto de dados vazio para tamanho " << current_size << std::endl;
            continue;
        }

        Structure ds_instance; // Nova instância para cada tamanho
        long long duration_insert_ns = measure_insertion(ds_instance, data_subset);
        long mem_after_insert = get_current_memory_usage_kb();
        results.emplace_back(structure_name, "insert_all", current_size, duration_insert_ns, mem_after_insert, current_scenario_name);
        std::cout << "    Insercao (" << current_size << " elems): " << duration_insert_ns / 1e6 << " ms, Mem: " << mem_after_insert << " KB" << std::endl;

        int num_ops_search_remove = std::max(1, current_size / 10);
        std::vector<EarthquakeRecord> specific_ops_data;
        if (!data_subset.empty()) {
            int count = 0;
            for(const auto& rec : data_subset){ // Pega os primeiros N elementos do subset atual
                specific_ops_data.push_back(rec);
                count++;
                if(count >= num_ops_search_remove) break;
            }
        }
        if (specific_ops_data.empty() && !data_subset.empty()){
            specific_ops_data.push_back(data_subset.front());
        }


        if(!specific_ops_data.empty()){
            long long duration_search_spec_ns = measure_search_specific(ds_instance, specific_ops_data); // Usa ds_instance já populada
            results.emplace_back(structure_name, "search_specific_existing", specific_ops_data.size(), duration_search_spec_ns, mem_after_insert, current_scenario_name);
            std::cout << "    Busca Espec. (existentes, " << specific_ops_data.size() << " ops): " << duration_search_spec_ns / 1e6 << " ms (media por op)" << std::endl;
        }

        long long duration_search_rand_ns = measure_search_random(ds_instance, num_ops_search_remove); // Usa ds_instance já populada
        results.emplace_back(structure_name, "search_random_existing", num_ops_search_remove, duration_search_rand_ns, mem_after_insert, current_scenario_name);
        std::cout << "    Busca Aleat. (existentes, " << num_ops_search_remove << " ops): " << duration_search_rand_ns / 1e6 << " ms (media por op)" << std::endl;

        if(!specific_ops_data.empty()){
            // Para teste de remoção, é melhor ter uma estrutura "fresca" com os mesmos dados
            // para não depender do estado após as buscas, e para que a remoção seja de elementos que existem.
            Structure ds_for_removal;
            for(const auto& rec : data_subset) ds_for_removal.insert_record(rec); // Popula com o mesmo subset

            long long duration_remove_ns = measure_removal(ds_for_removal, specific_ops_data);
            long mem_after_remove = get_current_memory_usage_kb(); // Memória após as remoções (pode não mudar muito)
            results.emplace_back(structure_name, "remove_specific_existing", specific_ops_data.size(), duration_remove_ns, mem_after_remove, current_scenario_name);
            std::cout << "    Remocao (existentes, " << specific_ops_data.size() << " ops): " << duration_remove_ns / 1e6 << " ms, Mem: " << mem_after_remove << " KB" << std::endl;
        }
        std::cout << "    ------------------------------------" << std::endl;
    }
}

void BenchmarkRunner::run_all_benchmarks() {
    results.clear();
    std::cout << "\n========= INICIANDO BENCHMARKS (Cenario: " << current_scenario_name << ") =========" << std::endl;

    // Lista Duplamente Ligada
    DoublyLinkedList dll_template; // Apenas para passar como argumento template
    benchmark_structure<DoublyLinkedList>(dll_template, "DoublyLinkedList");

    // Árvore AVL
    AVLTree avl_template; // Apenas para passar como argumento template
    benchmark_structure<AVLTree>(avl_template, "AVLTree");


    // Tabela Hash
    /*
    HashTable ht_template;
    benchmark_structure<HashTable>(ht_template, "HashTable");
    */

    // ... outras estruturas ...

    std::cout << "========= BENCHMARKS CONCLUIDOS (Cenario: " << current_scenario_name << ") =========" << std::endl;
}

void BenchmarkRunner::save_results_to_csv(const std::string& filename) const {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Erro ao abrir arquivo para salvar resultados do benchmark: " << filename << std::endl;
        return;
    }

    outfile << "DataStructure,Operation,InputSize,Duration_ns,MemoryUsage_KB,Scenario\n";

    for (const auto& result : results) {
        outfile << result.data_structure_name << ","
                << result.operation_type << ","
                << result.input_size << ","
                << result.duration_ns << ","
                << result.memory_usage_kb << ","
                << result.restriction_scenario << "\n";
    }

    outfile.close();
    std::cout << "Resultados do benchmark salvos em: " << filename << std::endl;
}