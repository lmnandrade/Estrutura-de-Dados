#include "statistic.h"
#include "csv_reader.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

// Conversor seguro
bool toFloatSafe(const std::string& str, float& out) {
    try {
        out = std::stof(str);
        return true;
    } catch (...) {
        return false;
    }
}

// Cálculos
float calcularMedia(const std::vector<float>& valores) {
    if (valores.empty()) return 0.0f;
    float soma = 0.0f;
    for (float v : valores) soma += v;
    return soma / valores.size();
}

float calcularMediana(std::vector<float> valores) {
    if (valores.empty()) return 0.0f;
    std::sort(valores.begin(), valores.end());
    size_t n = valores.size();
    return (n % 2 == 0) ? (valores[n / 2 - 1] + valores[n / 2]) / 2.0f : valores[n / 2];
}

float calcularVariancia(const std::vector<float>& valores) {
    if (valores.size() < 2) return 0.0f;
    float media = calcularMedia(valores);
    float soma = 0.0f;
    for (float v : valores) soma += (v - media) * (v - media);
    return soma / (valores.size() - 1);
}

float calcularDesvioPadrao(const std::vector<float>& valores) {
    return std::sqrt(calcularVariancia(valores));
}

// Extrator
std::vector<float> extrairColunaNumerica(const std::vector<EarthquakeRecord>& registros, const std::string& campo) {
    std::vector<float> valores;
    for (const auto& r : registros) {
        float val;
        if (campo == "Latitude" && toFloatSafe(r.latitude, val)) valores.push_back(val);
        else if (campo == "Longitude" && toFloatSafe(r.longitude, val)) valores.push_back(val);
        else if (campo == "Magnitude" && toFloatSafe(r.magnitude, val)) valores.push_back(val);
        else if (campo == "Profundidade" && toFloatSafe(r.depth, val)) valores.push_back(val);
        else if (campo == "Impact Score" && toFloatSafe(r.impact_score, val)) valores.push_back(val);
    }
    return valores;
}

// Menu principal com loop e pergunta ao final
void menu_estatisticas(const std::vector<EarthquakeRecord>& registros) {
    while (true) {
        int escolha;
        std::cout << "\n=== Análises Estatísticas ===\n";
        std::cout << "Escolha uma variável para análise:\n";
        std::cout << "1. Latitude\n";
        std::cout << "2. Longitude\n";
        std::cout << "3. Magnitude\n";
        std::cout << "4. Profundidade\n";
        std::cout << "5. Impact Score\n";
        std::cout << "0. Voltar ao Menu Principal\n";
        std::cout << "Escolha: ";
        std::cin >> escolha;

        std::string campo;
        switch (escolha) {
            case 1: campo = "Latitude"; break;
            case 2: campo = "Longitude"; break;
            case 3: campo = "Magnitude"; break;
            case 4: campo = "Profundidade"; break;
            case 5: campo = "Impact Score"; break;
            case 0:
                std::cout << "Retornando ao menu principal...\n";
                return;
            default:
                std::cout << "Opção inválida. Tente novamente.\n";
                continue;
        }

        std::vector<float> valores = extrairColunaNumerica(registros, campo);

        if (valores.empty()) {
            std::cout << "Não há dados válidos para a variável '" << campo << "'.\n";
        } else {
            std::cout << "\n--- Estatísticas para " << campo << " ---\n";
            std::cout << "Média: " << calcularMedia(valores) << "\n";
            std::cout << "Mediana: " << calcularMediana(valores) << "\n";
            std::cout << "Desvio Padrão: " << calcularDesvioPadrao(valores) << "\n";
            std::cout << "Variância: " << calcularVariancia(valores) << "\n";
        }

        // Perguntar se quer continuar
        std::string continuar;
        std::cout << "\nDeseja analisar outra variável? (s/n): ";
        std::cin >> continuar;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Limpa buffer

        // Remove espaços e converte para minúsculo
        continuar.erase(std::remove_if(continuar.begin(), continuar.end(), ::isspace), continuar.end());
        std::transform(continuar.begin(), continuar.end(), continuar.begin(), ::tolower);

        if (continuar != "s") {
            std::cout << "Retornando ao menu principal...\n";
            break;
        }

    }
}