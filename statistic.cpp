#include "statistic.h"
// #include "csv_reader.h" // Já incluído via statistic.h se EarthquakeRecord estiver lá

#include <iostream>
#include <vector>
#include <cmath>       // Para std::sqrt, std::pow
#include <algorithm>   // Para std::sort, std::remove_if, std::transform
#include <limits>      // Para std::numeric_limits
#include <map>         // Para calcular a moda
#include <iomanip>     // Para std::fixed e std::setprecision

// Conversor seguro de string para float
bool stringToFloatSafe(const std::string& str, float& out) {
    try {
        if (str.empty()) return false; // std::stof pode dar erro com string vazia dependendo da lib C++
        size_t processed_chars = 0;
        out = std::stof(str, &processed_chars);
        // Verifica se todos os caracteres foram processados (exceto espaços em branco no final, que stof pode ignorar)
        // Para ser mais rigoroso:
        // return processed_chars == str.length();
        return true; // Simplesmente retorna true se não houver exceção
    } catch (const std::invalid_argument& ia) {
        // std::cerr << "Erro de conversao (argumento invalido): " << str << std::endl;
        return false;
    } catch (const std::out_of_range& oor) {
        // std::cerr << "Erro de conversao (fora do range): " << str << std::endl;
        return false;
    } catch (...) { // Outras exceções desconhecidas
        return false;
    }
}

// Conversor seguro de string para int
bool stringToIntSafe(const std::string& str, int& out) {
    try {
        if (str.empty()) return false;
        size_t processed_chars = 0;
        out = std::stoi(str, &processed_chars);
        return true;
    } catch (...) {
        return false;
    }
}


// --- Funções de Cálculo Estatístico ---

float calcularMedia(const std::vector<float>& valores) {
    if (valores.empty()) return 0.0f;
    float soma = 0.0f;
    for (float v : valores) {
        soma += v;
    }
    return soma / valores.size();
}

float calcularMediana(std::vector<float> valores) { // Passa por valor para poder ordenar
    if (valores.empty()) return 0.0f;
    std::sort(valores.begin(), valores.end());
    size_t n = valores.size();
    if (n % 2 == 0) {
        return (valores[n / 2 - 1] + valores[n / 2]) / 2.0f;
    } else {
        return valores[n / 2];
    }
}

float calcularVariancia(const std::vector<float>& valores) {
    if (valores.size() < 2) return 0.0f; // Variância não definida para menos de 2 amostras (ou 0 para 1 amostra)
    float media = calcularMedia(valores);
    float soma_quadrados_diff = 0.0f;
    for (float v : valores) {
        soma_quadrados_diff += (v - media) * (v - media);
    }
    // Usando (N-1) para variância amostral (mais comum)
    return soma_quadrados_diff / (valores.size() - 1);
}

float calcularDesvioPadrao(const std::vector<float>& valores) {
    if (valores.size() < 2) return 0.0f;
    return std::sqrt(calcularVariancia(valores));
}

float calcularAssimetria(const std::vector<float>& valores) { // Skewness
    if (valores.size() < 3) return 0.0f; // Assimetria geralmente requer pelo menos 3 pontos
    float media = calcularMedia(valores);
    float desvio_padrao = calcularDesvioPadrao(valores);

    if (desvio_padrao == 0.0f) return 0.0f; // Evita divisão por zero se todos os valores forem iguais

    float soma_cubos_diff = 0.0f;
    for (float v : valores) {
        soma_cubos_diff += std::pow(v - media, 3);
    }
    
    // Fórmula para assimetria amostral (ajustada) é mais complexa.
    // Usando uma fórmula mais simples (populacional ou não ajustada para amostra pequena):
    // G1 = [n / ((n-1)*(n-2))] * sum((xi - mean)/stddev)^3 (para amostra)
    // g1 = sum((xi - mean)/stddev)^3 / n (para população)
    // Para simplificar, usaremos a g1, que é o momento central de terceira ordem normalizado.
    float n = static_cast<float>(valores.size());
    float m3 = soma_cubos_diff / n; // Terceiro momento central
    return m3 / std::pow(desvio_padrao, 3);
}

float calcularCurtose(const std::vector<float>& valores) { // Kurtosis
    if (valores.size() < 4) return 0.0f; // Curtose geralmente requer pelo menos 4 pontos
    float media = calcularMedia(valores);
    float desvio_padrao = calcularDesvioPadrao(valores);

    if (desvio_padrao == 0.0f) return 0.0f;

    float soma_quartas_diff = 0.0f;
    for (float v : valores) {
        soma_quartas_diff += std::pow(v - media, 4);
    }

    // Similar à assimetria, há fórmulas populacionais e amostrais (ajustadas).
    // g2 = sum((xi - mean)/stddev)^4 / n - 3 (Curtose de Excesso Populacional)
    // G2 é mais complexa para amostra.
    // Vamos calcular o quarto momento normalizado e depois o excesso.
    float n = static_cast<float>(valores.size());
    float m4 = soma_quartas_diff / n; // Quarto momento central
    float curtose_bruta = m4 / std::pow(desvio_padrao, 4);
    return curtose_bruta; // Retorna a curtose bruta. Curtose de excesso = curtose_bruta - 3
                          // A interpretação comum (leptocúrtica, mesocúrtica, platicúrtica) usa a curtose de excesso.
}

std::vector<float> calcularModa(const std::vector<float>& valores) {
    std::vector<float> modas;
    if (valores.empty()) return modas;

    std::map<float, int> contagens;
    for (float v : valores) {
        contagens[v]++;
    }

    int max_contagem = 0;
    for (const auto& par : contagens) {
        if (par.second > max_contagem) {
            max_contagem = par.second;
        }
    }

    // Se todas as contagens forem 1 (ou se a max_contagem for 1 e houver mais de um valor),
    // não há uma moda clara ou todos são modas.
    if (max_contagem <= 1 && valores.size() > 1 && contagens.size() == valores.size()) {
         // Todos os valores são únicos, ou a frequência máxima é 1 (sem moda clara)
         // Para este caso, podemos retornar um vetor vazio ou todos os valores
         // Retornar vazio indica "sem moda distinta"
        return modas;
    }


    for (const auto& par : contagens) {
        if (par.second == max_contagem) {
            modas.push_back(par.first);
        }
    }
    std::sort(modas.begin(), modas.end()); // Opcional: ordenar as modas se houver múltiplas
    return modas;
}

// --- Extrator de Coluna Numérica (Modificado para incluir Data) ---
std::vector<float> extrairColunaNumerica(const std::vector<EarthquakeRecord>& registros, const std::string& campo) {
    std::vector<float> valores;
    for (const auto& r : registros) {
        float val_float;
        int val_int;

        if (campo == "Latitude" && stringToFloatSafe(r.latitude, val_float)) valores.push_back(val_float);
        else if (campo == "Longitude" && stringToFloatSafe(r.longitude, val_float)) valores.push_back(val_float);
        else if (campo == "Magnitude" && stringToFloatSafe(r.magnitude, val_float)) valores.push_back(val_float);
        else if (campo == "Profundidade" && stringToFloatSafe(r.depth, val_float)) valores.push_back(val_float);
        else if (campo == "Impact Score" && stringToFloatSafe(r.impact_score, val_float)) valores.push_back(val_float);
        else if (campo == "Ano" || campo == "Mes" || campo == "Dia") {
            if (r.date.length() >= 10) { // "AAAA-MM-DD"
                std::string ano_str = r.date.substr(0, 4);
                std::string mes_str = r.date.substr(5, 2);
                std::string dia_str = r.date.substr(8, 2);

                if (campo == "Ano" && stringToIntSafe(ano_str, val_int)) valores.push_back(static_cast<float>(val_int));
                else if (campo == "Mes" && stringToIntSafe(mes_str, val_int)) valores.push_back(static_cast<float>(val_int));
                else if (campo == "Dia" && stringToIntSafe(dia_str, val_int)) valores.push_back(static_cast<float>(val_int));
            }
        }
    }
    return valores;
}


// --- Menu Principal de Estatísticas (Modificado) ---
void menu_estatisticas(const std::vector<EarthquakeRecord>& registros) {
    if (registros.empty()) {
        std::cout << "Nao ha registros para analisar estatisticamente." << std::endl;
        return;
    }
    
    std::cout << std::fixed << std::setprecision(4); // Define precisão para floats

    while (true) {
        int escolha;
        std::cout << "\n=== Análises Estatísticas ===\n";
        std::cout << "Escolha uma variável para análise:\n";
        std::cout << "1. Latitude\n";
        std::cout << "2. Longitude\n";
        std::cout << "3. Magnitude\n";
        std::cout << "4. Profundidade\n";
        std::cout << "5. Impact Score\n";
        std::cout << "6. Ano do Terremoto\n";
        std::cout << "7. Mes do Terremoto\n";
        std::cout << "8. Dia do Terremoto\n";
        std::cout << "0. Voltar ao Menu Principal\n";
        std::cout << "Escolha: ";
        
        if (!(std::cin >> escolha)) {
            std::cout << "Entrada invalida. Por favor, insira um numero.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Limpa o buffer após ler o número

        std::string campo_selecionado;
        switch (escolha) {
            case 1: campo_selecionado = "Latitude"; break;
            case 2: campo_selecionado = "Longitude"; break;
            case 3: campo_selecionado = "Magnitude"; break;
            case 4: campo_selecionado = "Profundidade"; break;
            case 5: campo_selecionado = "Impact Score"; break;
            case 6: campo_selecionado = "Ano"; break;
            case 7: campo_selecionado = "Mes"; break;
            case 8: campo_selecionado = "Dia"; break;
            case 0:
                std::cout << "Retornando ao menu principal...\n";
                return;
            default:
                std::cout << "Opção inválida. Tente novamente.\n";
                continue;
        }

        std::vector<float> valores = extrairColunaNumerica(registros, campo_selecionado);

        if (valores.empty()) {
            std::cout << "\nNao ha dados numericos validos para a variavel '" << campo_selecionado << "' ou a variavel nao foi encontrada.\n";
        } else {
            float media_val = calcularMedia(valores);
            float mediana_val = calcularMediana(valores); // calcularMediana recebe por valor e ordena
            float desvio_padrao_val = calcularDesvioPadrao(valores);
            float variancia_val = calcularVariancia(valores);
            float assimetria_val = calcularAssimetria(valores);
            float curtose_val = calcularCurtose(valores); // Curtose bruta
            std::vector<float> moda_val = calcularModa(valores);

            std::cout << "\n--- Análise da Coluna: " << campo_selecionado << " (" << valores.size() << " valores validos) ---" << std::endl;
            std::cout << "Coeficiente de Assimetria: " << assimetria_val << std::endl;
            std::cout << "Coeficiente de Curtose: " << curtose_val << " (Curtose de Excesso: " << (curtose_val - 3.0f) << ")" << std::endl;
            std::cout << "Desvio Padrão: " << desvio_padrao_val << std::endl;
            std::cout << "Variância: " << variancia_val << std::endl;
            std::cout << "--------------------------------------------------" << std::endl;
            std::cout << "Medidas de Centralidade Calculadas:" << std::endl;
            std::cout << "Média: " << media_val << std::endl;
            std::cout << "Mediana: " << mediana_val << std::endl;
            std::cout << "Moda: ";
            if (moda_val.empty()) {
                std::cout << "Nao ha moda distinta (ou todos os valores sao unicos)." << std::endl;
            } else {
                for (size_t i = 0; i < moda_val.size(); ++i) {
                    std::cout << moda_val[i] << (i < moda_val.size() - 1 ? ", " : "");
                }
                std::cout << std::endl;
            }
        }

        std::string continuar_str;
        std::cout << "\nDeseja realizar outra analise estatistica? (s/n): ";
        std::getline(std::cin, continuar_str); // Use getline para evitar problemas com espaços

        // Remove espaços e converte para minúsculo
        continuar_str.erase(std::remove_if(continuar_str.begin(), continuar_str.end(), ::isspace), continuar_str.end());
        std::transform(continuar_str.begin(), continuar_str.end(), continuar_str.begin(), ::tolower);

        if (continuar_str != "s" && continuar_str != "sim") {
            std::cout << "Retornando ao menu principal...\n";
            break; 
        }
    }
}