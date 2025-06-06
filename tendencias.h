#ifndef TENDENCIAS_H
#define TENDENCIAS_H

#include "csv_reader.h" // Para EarthquakeRecord (embora não seja usado para treinar, pode ser para contexto)
#include <vector>
#include <string>

// Estrutura para armazenar os parâmetros do modelo de regressão linear pré-calculado
struct PredefinedLinearModel {
    double b0;          // Intercepto (valor de y quando x é 0)
    double b1;          // Inclinação (quanto y muda para uma unidade de mudança em x)
    double r_squared;   // Coeficiente de Determinação (R²)
    double mse;         // Erro Quadrático Médio (MSE)
    std::string formula_str; // String da fórmula para exibição

    // Construtor para inicializar com os valores do Python
    PredefinedLinearModel(double intercept, double slope, double r2, double mean_sq_err, const std::string& eq_str)
        : b0(intercept), b1(slope), r_squared(r2), mse(mean_sq_err), formula_str(eq_str) {}
};

// Função principal do menu de tendências e previsões usando um modelo pré-definido
// all_records pode ser passado para contexto ou se você quiser mostrar alguma estatística dos dados originais.
void show_trends_menu_with_predefined_model(
    const std::vector<EarthquakeRecord>& all_records,
    const PredefinedLinearModel& model
);

#endif // TENDENCIAS_H