#include "benchmark.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif
// ...restante do código...
// Headers das estruturas de dados (já devem estar incluídos via benchmark.h)

// Implementação do Construtor
BenchmarkRunner::BenchmarkRunner(const std::vector<EarthquakeRecord>& all_data, const std::string& scenario_name)
    : full_dataset(all_data), current_scenario_name(scenario_name) {
    if (all_data.empty()) {
        std::cout << "AVISO (BenchmarkRunner): O dataset original fornecido esta vazio. Os benchmarks podem usar dados gerados ou subconjuntos limitados." << std::endl;
    }
     std::cout << "BenchmarkRunner inicializado para o cenario: " << current_scenario_name
              << " com " << full_dataset.size() << " registros no dataset principal." << std::endl;
}

void BenchmarkRunner::run_all_benchmarks() {
    results.clear(); // Limpa resultados de execuções anteriores
    std::cout << "\n========= INICIANDO BENCHMARKS (Cenario: " << current_scenario_name << ") =========" << std::endl;

    benchmark_structure<DoublyLinkedList>("DoublyLinkedList");
    benchmark_structure<AVLTree>("AVLTree");
    benchmark_structure<KDTree>("KDTree");
    benchmark_structure<HashTable>("HashTable");
    benchmark_structure<SkipList>("SkipList");

    std::cout << "========= BENCHMARKS CONCLUIDOS (Cenario: " << current_scenario_name << ") =========" << std::endl;
}

void BenchmarkRunner::save_results_to_csv(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERRO: Nao foi possivel abrir o arquivo para salvar os resultados do benchmark: " << filename << std::endl;
        return;
    }

    file << "DataStructure,Operation,InputSize,Duration_ns,MemoryUsage_KB_Delta,Scenario\n";

    for (const auto& r : results) {
        file << r.data_structure_name << ","
             << r.operation_type << ","
             << r.input_size << ","
             << r.duration_ns << ","
             << r.memory_usage_kb << ","
             << r.restriction_scenario << "\n";
    }

    file.close();
    std::cout << "Resultados do benchmark salvos em: " << filename << std::endl;
}

std::vector<EarthquakeRecord> BenchmarkRunner::get_subset_of_data(int size) const {
    if (full_dataset.empty()) {
        return {};
    }
    if (size <= 0) {
        return {};
    }

    std::vector<EarthquakeRecord> subset;
    if (static_cast<size_t>(size) >= full_dataset.size()) {
        subset = full_dataset;
    } else {
        subset.assign(full_dataset.begin(), full_dataset.begin() + size);
    }
    return subset;
}

// Medição de memória (implementação para Windows e fallback)
long BenchmarkRunner::get_current_memory_usage_kb() {
    #ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc;
        ZeroMemory(&pmc, sizeof(pmc)); // É uma boa prática zerar a estrutura
        pmc.cb = sizeof(pmc);          // cb deve ser setado para o tamanho da estrutura
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            return static_cast<long>(pmc.PrivateUsage / 1024); // Uso privado em KB
        } else {
            // std::cerr << "AVISO (BenchmarkRunner): GetProcessMemoryInfo falhou. Codigo de erro: " << GetLastError() << std::endl;
            return 0;
        }
    #else
        // Fallback para plataformas não-Windows: Medição de memória não implementada desta forma.
        // std::cout << "AVISO (BenchmarkRunner): Medicao de memoria detalhada nao implementada para esta plataforma. Retornando 0." << std::endl;
        return 0; // Retorna 0 se não for Windows e não houver outra implementação
    #endif
}

EarthquakeRecord BenchmarkRunner::get_random_record_from_subset(const std::vector<EarthquakeRecord>& subset) const {
    if (subset.empty()) {
        std::cerr << "AVISO (BenchmarkRunner): Tentativa de obter registro aleatorio de um subset vazio!" << std::endl;
        // Retorna um registro "vazio" ou padrão. Certifique-se de que EarthquakeRecord tenha um construtor padrão adequado.
        return EarthquakeRecord{"N/A", "N/A", "0", "0", "N/A", "N/A", "0", "0", "0"};
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, static_cast<int>(subset.size() - 1));
    return subset[distrib(gen)];
}

// NOTA: As implementações dos métodos template (benchmark_structure, measure_insertion, etc.)
// devem estar em benchmark.h ou em um arquivo .tpp/.ipp incluído por benchmark.h.