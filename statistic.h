#ifndef ESTATISTICA_H
#define ESTATISTICA_H

#include "csv_reader.h"
#include <vector>
#include <string>

float calcularMedia(const std::vector<float>& valores);
float calcularMediana(std::vector<float> valores);
float calcularDesvioPadrao(const std::vector<float>& valores);
float calcularVariancia(const std::vector<float>& valores);

void menu_estatisticas(const std::vector<EarthquakeRecord>& registros);

#endif // ESTATISTICA_H