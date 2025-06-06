#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "csv_reader.h"
#include "benchmark.h"

inline void benchmark_menu(const std::vector<EarthquakeRecord>& dataset) {
    BenchmarkRunner runner(dataset, "Benchmark Principal");

    int opcao = 0;
    do {
        std::cout << "\n=== MENU DE BENCHMARK ===\n";
        std::cout << "1 - Benchmarks padrão e restritos\n";
        std::cout << "2 - Testes de escalabilidade\n";
        std::cout << "3 - Teste: Limite de tamanho máximo\n";
        std::cout << "4 - Teste: Simulação de concorrência\n";
        std::cout << "5 - Teste: Alta latência de IO\n";
        std::cout << "6 - Teste: Chegada irregular de dados\n";
        std::cout << "7 - Teste: Reindexação frequente\n";
        std::cout << "8 - Salvar resultados em CSV\n";
        std::cout << "0 - Voltar\n";
        std::cout << "Opcao: ";
        std::cin >> opcao;

        switch (opcao) {
            case 1:
                runner.run_all_benchmarks_and_restricted_tests();
                break;
            case 2:
                runner.run_scalability_tests();
                break;
            case 3:
                runner.run_max_size_restriction_tests(10000); // Exemplo de parâmetro
                break;
            case 4:
                runner.run_concurrency_simulation_tests(std::chrono::milliseconds(5), 20);
                break;
            case 5:
                runner.run_high_latency_io_tests(std::chrono::milliseconds(10));
                break;
            case 6:
                runner.run_irregular_arrival_tests(std::chrono::milliseconds(10));
                break;
            case 7:
                runner.run_frequent_reindex_tests();
                break;
            case 8:
                runner.save_results_to_csv("benchmark_results.csv");
                break;
            case 0:
                std::cout << "Voltando ao menu principal.\n";
                break;
            default:
                std::cout << "Opcao invalida!\n";
        }
    } while (opcao != 0);
}