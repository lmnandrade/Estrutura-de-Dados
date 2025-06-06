#include "tendencias.h"
#include <iostream>
#include <vector>
#include <limits>    // Para std::numeric_limits
#include <iomanip>   // Para std::fixed, std::setprecision

// Não precisamos mais de:
// - stringToDoubleSafe (a menos que a entrada do usuário precise)
// - prepare_regression_data
// - train_linear_regression_with_metrics

void show_trends_menu_with_predefined_model(
    const std::vector<EarthquakeRecord>& all_records, // Passado para possível uso futuro ou contexto
    const PredefinedLinearModel& model)
{
    std::cout << "\n===== Previsao de Tendencias (Modelo de Regressao Pre-calculado) =====" << std::endl;
    std::cout << "Usando modelo e metricas fornecidas externamente (ex: Python)." << std::endl;

    std::cout << std::fixed << std::setprecision(4); // Define a formatação de saída para doubles

    std::cout << "\n--- Detalhes do Modelo Fornecido ---" << std::endl;
    std::cout << "  Formula: " << model.formula_str << std::endl;
    std::cout << "  Intercepto (b0): " << model.b0 << std::endl;
    std::cout << "  Inclinacao (b1): " << model.b1 << std::endl;
    std::cout << "  Coeficiente de Determinacao (R²): " << model.r_squared << std::endl;
    std::cout << "  Erro Quadratico Medio (MSE):    " << model.mse << std::endl;

    // Interpretação de R² (mesma lógica de antes)
    if (model.r_squared >= 0.7) {
        std::cout << "  Interpretacao R²: O modelo explica aproximadamente " << model.r_squared * 100 << "% da variabilidade no Impact Score (ajuste forte)." << std::endl;
    } else if (model.r_squared >= 0.4) {
        std::cout << "  Interpretacao R²: O modelo explica aproximadamente " << model.r_squared * 100 << "% da variabilidade no Impact Score (ajuste moderado)." << std::endl;
    } else if (model.r_squared >= 0.1) {
        std::cout << "  Interpretacao R²: O modelo explica aproximadamente " << model.r_squared * 100 << "% da variabilidade no Impact Score (ajuste fraco)." << std::endl;
    } else {
        std::cout << "  Interpretacao R²: O modelo explica muito pouco (" << model.r_squared * 100 << "%) da variabilidade no Impact Score (ajuste muito fraco ou inexistente)." << std::endl;
    }

    if (model.b1 < 0 && model.r_squared > 0.05) {
        std::cout << "  AVISO: A inclinacao (b1 = " << model.b1 << ") do modelo fornecido e negativa." << std::endl;
        std::cout << "  Isso sugere que, de acordo com este modelo, um aumento na magnitude leva a uma DIMINUICAO no Impact Score." << std::endl;
        std::cout << "  Se isso for contraintuitivo, verifique a analise em Python e os dados usados." << std::endl;
    } else if (model.b1 >= 0) {
         std::cout << "  Interpretacao Inclinacao (b1): Em media, para cada aumento de 1 unidade na magnitude, o impact score muda em " << model.b1 << " unidades, de acordo com este modelo." << std::endl;
    }
    std::cout << "  (Lembre-se: este modelo e uma representacao dos dados analisados externamente.)" << std::endl;


    char choice_char;
    do {
        std::cout << "\nDeseja fazer uma previsao de Impact Score para uma magnitude estimada usando este modelo? (s/n): ";
        if (!(std::cin >> choice_char)) {
             std::cout << "Erro de leitura. Tente novamente." << std::endl;
             std::cin.clear();
             std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
             choice_char = ' ';
             continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice_char == 's' || choice_char == 'S') {
            double estimated_magnitude;
            std::cout << "Digite a magnitude estimada: ";
            if (!(std::cin >> estimated_magnitude)) {
                std::cout << "Entrada invalida para magnitude." << std::endl;
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // Previsão usando os coeficientes do modelo fornecido
            double predicted_impact_score = model.b0 + model.b1 * estimated_magnitude;

            std::cout << "Para uma magnitude de " << estimated_magnitude
                      << ", o Impact Score previsto (usando o modelo fornecido) e: " << predicted_impact_score << std::endl;

            if (predicted_impact_score < 0) {
                std::cout << "  (AVISO: A previsao resultou em um valor negativo. Considere se isso e esperado para Impact Score.)" << std::endl;
            }

        } else if (choice_char != 'n' && choice_char != 'N') {
            std::cout << "Opcao invalida. Por favor, digite 's' ou 'n'." << std::endl;
        }
    } while (choice_char != 'n' && choice_char != 'N');

    std::cout << "Retornando ao menu principal..." << std::endl;
}