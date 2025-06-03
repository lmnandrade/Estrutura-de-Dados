#ifndef LISTA_FUNCTIONS_H
#define LISTA_FUNCTIONS_H

#include "lista_dupla_ligada.h"
#include <string>
#include <chrono>
#include <vector>

// Estrutura para armazenar resultados de benchmark
struct BenchmarkResult {
    double insertionTime;
    double searchTime;
    double removalTime;
};

// Funções de benchmark
BenchmarkResult runBenchmark(ListaDuplaLigada& lista, const std::vector<EarthquakeRecord>& records);

// Função para manipular a lista duplamente encadeada (menu)
void menuListaDupla(ListaDuplaLigada& lista);

#endif // LISTA_FUNCTIONS_H