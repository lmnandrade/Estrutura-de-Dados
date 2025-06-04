#include "benchmark.h"
#include <iostream>
#include <algorithm> // Para std::shuffle, std::sample
#include <random>    // Para gerador de números aleatórios
#include <iomanip>   // Para std::fixed, std::setprecision

#ifdef __linux__ // Para getrusage no Linux
#include <sys/resource.h>
#endif

BenchmarkRunner::BenchmarkRunner(const std::vector<EarthquakeRecord>& all_data, const std::string& scenario_name)
    : dataset(all_data), current_scenario_name(scenario_name) {
    if (dataset.empty()) {
        std::cerr << "AVISO (Benchmark): Dataset vazio fornecido para benchmarking." << std::endl;
    }
}

// Função para obter uma subamostra do dataset
std::vector<EarthquakeRecord> BenchmarkRunner::get_subset_of_data(int size) const {
    if (size <= 0 || dataset.empty()) return {};
    if (static_cast<size_t>(size) >= dataset.size()) return dataset; // Retorna tudo se o tamanho pedido for maior

    std::vector<EarthquakeRecord> subset;
    std::sample(dataset.begin(), dataset.end(), std::back_inserter(subset),
                size, std::mt19937{std::random_device{}()});
    return subset;
}

long BenchmarkRunner::get_current_memory_usage_kb() {
    // Esta função é ALTAMENTE dependente da plataforma e do contexto (Docker).
    // Dentro do Docker, você pode não conseguir obter isso de forma confiável internamente.
    // O ideal é usar 'docker stats' externamente e correlacionar.
    // Para testes locais no Linux, getrusage pode dar uma ideia.

#ifdef __linux__
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        // ru_maxrss está em kilobytes no Linux
        return usage.ru_maxrss;
    } else {
        std::cerr << "Erro ao obter uso de memoria com getrusage." << std::endl;
        return -1; // Erro
    }
#else
    // Para Windows/macOS, você precisaria de APIs específicas ou ferramentas externas.
    // std::cout << "AVISO: Medição de memória interna não implementada para esta plataforma." << std::endl;
    return 0; // Retorna 0 ou -1 para indicar que não foi medido internamente
#endif
}


template<typename Structure>
long long BenchmarkRunner::measure_insertion(Structure& ds, const std::vector<EarthquakeRecord>& records_to_insert) {
    auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto& record : records_to_insert) {
        ds.insert_record(record); // Assumindo que todas as estruturas têm este método
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
}

template<typename Structure>
long long BenchmarkRunner::measure_search_specific(const Structure& ds, const std::vector<EarthquakeRecord>& records_to_search) {
    if (records_to_search.empty()) return 0;
    long long total_duration_ns = 0;
    
    // Para estruturas como lista/árvore, uma busca pode ser por um identificador.
    // Se a estrutura não tem uma busca por objeto completo, você precisa adaptar.
    // Exemplo para Lista Duplamente Ligada: buscar por cidade (ou outro critério).
    // Aqui, vamos simular a busca por um critério que existe em EarthquakeRecord
    // (ex: buscar pela data e cidade, esperando que o método de busca da estrutura suporte isso).

    // ESTA PARTE PRECISA DE ADAPTAÇÃO CONFORME OS MÉTODOS DE BUSCA DE CADA ESTRUTURA
    // Por exemplo, se a estrutura só busca por um ID único, você precisa gerar/usar esses IDs.
    // Para simplificar, vamos assumir que a estrutura pode tentar "encontrar" um registro
    // baseado em alguns de seus campos, mesmo que a busca não seja otimizada para isso.

    auto start_time_total = std::chrono::high_resolution_clock::now();
    for (const auto& record_criteria : records_to_search) {
        // Exemplo genérico:
        // auto found_records = ds.search_by_city(record_criteria.city); // Adapte ao método real
        // Ou, se a estrutura tiver um método 'find' que retorna um booleano ou iterador:
        // ds.find_record(record_criteria); // O método 'find_record' precisaria ser definido
        
        // Para DoublyLinkedList, podemos usar search_by_date_city_country (que não temos, mas podemos criar um critério)
        // Como a DLL só tem search_by_city, etc. separadamente, uma busca "específica" é mais complexa de generalizar aqui.
        // Vamos simular buscando pela cidade do primeiro registro a ser buscado.
        // ISTO É UMA GRANDE SIMPLIFICAÇÃO E PRECISA SER MELHORADO PARA CADA ESTRUTURA.
        if constexpr (std::is_same_v<Structure, DoublyLinkedList>) {
             auto result = ds.search_by_city(record_criteria.city); // Apenas um exemplo
        } else {
            // Adicione lógica específica para outras estruturas aqui
            // Ex: if constexpr (std::is_same_v<Structure, AVLTree>) { ds.search(key_of_record_criteria); }
        }
    }
    auto end_time_total = std::chrono::high_resolution_clock::now();
    total_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_total - start_time_total).count();

    return total_duration_ns / (records_to_search.empty() ? 1 : records_to_search.size()); // Duração média por busca
}


template<typename Structure>
long long BenchmarkRunner::measure_search_random(const Structure& ds, int num_searches) {
    if (num_searches <= 0 || ds.is_empty()) return 0; // Assumindo que ds.is_empty() existe

    // Para buscar aleatoriamente, precisamos de uma forma de pegar elementos aleatórios
    // que sabemos que ESTÃO na estrutura.
    // Uma forma é pegar uma amostra dos dados que foram inseridos.
    // Esta parte é complicada sem saber como cada estrutura armazena e permite acesso.

    // Simulação: Se a estrutura puder retornar todos os seus elementos:
    std::vector<EarthquakeRecord> all_in_ds; // = ds.get_all_records_vector(); // Se existir tal método
    
    // Se não puder, você precisa ter uma cópia dos dados inseridos e pegar amostras dela
    // para usar como critério de busca. Vamos assumir que 'dataset' contém os dados
    // que foram inseridos na estrutura.
    if (dataset.size() < static_cast<size_t>(num_searches)) {
        // std::cerr << "Aviso (Search Random): Dataset menor que o numero de buscas aleatorias." << std::endl;
        if(dataset.empty()) return 0;
        num_searches = dataset.size(); // Ajusta para o tamanho do dataset
    }

    std::vector<EarthquakeRecord> records_to_search_randomly;
    if (!dataset.empty()) {
        std::sample(dataset.begin(), dataset.end(), std::back_inserter(records_to_search_randomly),
                    num_searches, std::mt19937{std::random_device{}()});
    } else {
        return 0; // Não há o que buscar
    }
    
    // Reutiliza a lógica de measure_search_specific, mas com os dados aleatórios
    return measure_search_specific(ds, records_to_search_randomly);
}


template<typename Structure>
long long BenchmarkRunner::measure_removal(Structure& ds, const std::vector<EarthquakeRecord>& records_to_remove) {
    auto start_time = std::chrono::high_resolution_clock::now();
    for (const auto& record : records_to_remove) {
        // Assumindo que o método de remoção usa critérios do objeto EarthquakeRecord
        ds.remove_record(record.date, record.time, record.city, record.country);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count();
}


template<typename Structure>
void BenchmarkRunner::benchmark_structure(Structure& ds_template, const std::string& structure_name) {
    std::cout << "\n--- Benchmarking: " << structure_name << " (Cenario: " << current_scenario_name << ") ---" << std::endl;

    // Diferentes tamanhos de entrada para testar escalabilidade
    // Ajuste estes tamanhos conforme o tamanho do seu dataset completo
    std::vector<int> input_sizes;
    if (dataset.size() < 100) input_sizes = { (int)dataset.size() };
    else if (dataset.size() < 1000) input_sizes = {10, 50, (int)dataset.size() / 2, (int)dataset.size()};
    else if (dataset.size() < 10000) input_sizes = {100, 500, 1000, (int)dataset.size() / 2, (int)dataset.size()};
    else input_sizes = {1000, 5000, 10000, (int)dataset.size() / 2, (int)dataset.size()};
    
    // Remover duplicatas e garantir que o maior tamanho não exceda o dataset
    std::sort(input_sizes.begin(), input_sizes.end());
    input_sizes.erase(std::unique(input_sizes.begin(), input_sizes.end()), input_sizes.end());
    input_sizes.erase(std::remove_if(input_sizes.begin(), input_sizes.end(), 
                                   [&](int s){ return s <=0 || static_cast<size_t>(s) > dataset.size(); }), 
                      input_sizes.end());
    if (input_sizes.empty() && !dataset.empty()) {
        input_sizes.push_back(dataset.size());
    } else if (input_sizes.empty() && dataset.empty()){
        std::cout << "Dataset vazio, nenhum benchmark sera executado para " << structure_name << std::endl;
        return;
    }


    for (int current_size : input_sizes) {
        std::cout << "  Tamanho da Entrada: " << current_size << std::endl;
        
        // Preparar dados para este tamanho
        std::vector<EarthquakeRecord> data_subset = get_subset_of_data(current_size);
        if (data_subset.empty()) continue;

        // --- Teste de Inserção ---
        Structure ds_instance; // Cria uma nova instância para cada tamanho de teste de inserção
        long long duration_insert_ns = measure_insertion(ds_instance, data_subset);
        long mem_after_insert = get_current_memory_usage_kb(); // Memória após popular
        results.emplace_back(structure_name, "insert_all", current_size, duration_insert_ns, mem_after_insert, current_scenario_name);
        std::cout << "    Insercao (" << current_size << " elems): " << duration_insert_ns / 1e6 << " ms, Mem: " << mem_after_insert << " KB" << std::endl;

        // Para os próximos testes, usamos a ds_instance que já foi populada
        int num_ops_search_remove = std::max(1, current_size / 10); // Ex: 10% dos elementos para buscar/remover

        // Preparar dados para busca específica (ex: primeiros N da inserção) e remoção
        std::vector<EarthquakeRecord> specific_ops_data;
        if (!data_subset.empty()) {
            int count = 0;
            for(const auto& rec : data_subset){
                specific_ops_data.push_back(rec);
                count++;
                if(count >= num_ops_search_remove) break;
            }
        }
        if (specific_ops_data.empty() && !data_subset.empty()){ // Garante que temos algo para buscar/remover
            specific_ops_data.push_back(data_subset.front());
        }


        // --- Teste de Busca Específica ---
        if(!specific_ops_data.empty()){
            long long duration_search_spec_ns = measure_search_specific(ds_instance, specific_ops_data);
            results.emplace_back(structure_name, "search_specific", specific_ops_data.size(), duration_search_spec_ns, mem_after_insert, current_scenario_name);
            std::cout << "    Busca Espec. (" << specific_ops_data.size() << " ops): " << duration_search_spec_ns / 1e6 << " ms (media por op)" << std::endl;
        }

        // --- Teste de Busca Aleatória (elementos que existem) ---
        long long duration_search_rand_ns = measure_search_random(ds_instance, num_ops_search_remove);
        results.emplace_back(structure_name, "search_random_existing", num_ops_search_remove, duration_search_rand_ns, mem_after_insert, current_scenario_name);
        std::cout << "    Busca Aleat. (" << num_ops_search_remove << " ops): " << duration_search_rand_ns / 1e6 << " ms (media por op)" << std::endl;

        // --- Teste de Remoção ---
        // Para não afetar as buscas, podemos popular novamente ou usar uma cópia para remoção.
        // Ou, se a ordem não importar tanto para o benchmark, remover da instância atual.
        // Vamos remover da instância atual, mas os specific_ops_data foram baseados no data_subset original.
        if(!specific_ops_data.empty()){
            Structure ds_for_removal = ds_instance; // Copia a estrutura para o teste de remoção (se a cópia for barata)
                                                // Se a cópia for cara, precisa repopular.
                                                // Para DLL, cópia é cara. Melhor repopular.
            Structure ds_repopulated_for_removal;
            for(const auto& rec : data_subset) ds_repopulated_for_removal.insert_record(rec);

            long long duration_remove_ns = measure_removal(ds_repopulated_for_removal, specific_ops_data);
            long mem_after_remove = get_current_memory_usage_kb();
            results.emplace_back(structure_name, "remove_specific", specific_ops_data.size(), duration_remove_ns, mem_after_remove, current_scenario_name);
            std::cout << "    Remocao (" << specific_ops_data.size() << " ops): " << duration_remove_ns / 1e6 << " ms, Mem: " << mem_after_remove << " KB" << std::endl;
        }
        std::cout << "    ------------------------------------" << std::endl;
    }
}

void BenchmarkRunner::run_all_benchmarks() {
    results.clear();
    std::cout << "\n========= INICIANDO BENCHMARKS (Cenario: " << current_scenario_name << ") =========" << std::endl;

    // Lista Duplamente Ligada
    DoublyLinkedList dll;
    benchmark_structure<DoublyLinkedList>(dll, "DoublyLinkedList");

    // Árvore AVL (Exemplo, você precisará da sua implementação)
    /*
    AVLTree avl; // Supondo que você tenha AVLTree
    benchmark_structure<AVLTree>(avl, "AVLTree");
    */

    // Tabela Hash
    /*
    HashTable ht;
    benchmark_structure<HashTable>(ht, "HashTable");
    */

    // Skip List
    /*
    SkipList sl;
    benchmark_structure<SkipList>(sl, "SkipList");
    */
    
    // KD-Tree
    // O benchmark da KD-Tree pode precisar de lógica diferente,
    // especialmente para inserção (se ela se reconstrói) e busca (busca por proximidade).
    // Para buscas exatas, você pode adaptar.
    /*
    KDTree kdt;
    // benchmark_structure_kdtree(kdt, "KDTree"); // Pode precisar de uma função de benchmark especializada
    std::cout << "Benchmark para KD-Tree precisa de adaptacao especifica." << std::endl;
    */

    std::cout << "========= BENCHMARKS CONCLUIDOS (Cenario: " << current_scenario_name << ") =========" << std::endl;
}

void BenchmarkRunner::save_results_to_csv(const std::string& filename) const {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Erro ao abrir arquivo para salvar resultados do benchmark: " << filename << std::endl;
        return;
    }

    // Cabeçalho do CSV
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