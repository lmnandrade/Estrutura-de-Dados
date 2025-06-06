#ifndef ESTATISTICA_H
#define ESTATISTICA_H

#include "csv_reader.h" // Para EarthquakeRecord
#include <vector>
#include <string>

// Funções de cálculo existentes
float calcularMedia(const std::vector<float>& valores);
float calcularMediana(std::vector<float> valores); // Copia o vetor para ordená-lo
float calcularVariancia(const std::vector<float>& valores);
float calcularDesvioPadrao(const std::vector<float>& valores);

// Novas funções de cálculo
float calcularAssimetria(const std::vector<float>& valores); // Skewness
float calcularCurtose(const std::vector<float>& valores);    // Kurtosis
std::vector<float> calcularModa(const std::vector<float>& valores);

// Função principal do menu
void menu_estatisticas(const std::vector<EarthquakeRecord>& registros);

#endif // ESTATISTICA_H