#include "classification.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <numeric>   // Para std::accumulate (poderia ser usado para somar)
#include <stdexcept> // Para std::stod
#include <iomanip>   // Para std::fixed, std::setprecision

// Estrutura temporária para agregar dados por país antes da classificação final
// Definida aqui pois é um detalhe de implementação do .cpp
namespace { // Usando namespace anônimo para escopo local ao arquivo
    struct CountryAggregatedData {
        int total_earthquakes = 0;
        std::vector<double> magnitudes;
        std::vector<double> impact_scores;
    };
}

// Função auxiliar para tentar converter string para double
// Retorna true se sucesso, false caso contrário. O valor é passado por referência.


std::map<std::string, CountryRiskProfile> classify_countries_by_risk(
    const std::vector<EarthquakeRecord>& all_records) {
    
    std::map<std::string, CountryAggregatedData> aggregated_data_map;

    // 1. Agregar dados por país
    for (const auto& record : all_records) {
        if (record.country.empty()) {
            // std::cerr << "AVISO: Registro com nome de pais vazio ignorado (Data: " << record.date << ")." << std::endl;
            continue; // Ignora registros sem nome de país
        }

        // Limpar espaços em branco do nome do país para consistência
        std::string country_key = record.country;
        country_key.erase(0, country_key.find_first_not_of(" \t\n\r\f\v"));
        country_key.erase(country_key.find_last_not_of(" \t\n\r\f\v") + 1);
        if (country_key.empty()){
            // std::cerr << "AVISO: Registro com nome de pais apenas com espacos ignorado (Data: " << record.date << ")." << std::endl;
            continue;
        }


        aggregated_data_map[country_key].total_earthquakes++;
        
    }

    std::map<std::string, CountryRiskProfile> risk_profiles;

    // 2. Calcular métricas, pontuações e risco final para cada país
    for (const auto& pair : aggregated_data_map) {
        const std::string& country_name = pair.first;
        const CountryAggregatedData& agg_data = pair.second;
        CountryRiskProfile profile;
        profile.country_name = country_name;

        // Parâmetro 1: Número total de terremotos
        profile.total_earthquakes = agg_data.total_earthquakes;
        if (profile.total_earthquakes >= 101) {
            profile.score_param1_earthquakes = 3; // Alto
        } else if (profile.total_earthquakes >= 21) { // 21 a 100
            profile.score_param1_earthquakes = 2; // Médio
        } else { // 0 a 20
            profile.score_param1_earthquakes = 1; // Baixo (inclui 0 terremotos)
        }


        // Parâmetro 2: Média das magnitudes
        if (!agg_data.magnitudes.empty()) {
            double sum_mag = 0;
            for(double mag : agg_data.magnitudes) sum_mag += mag;
            profile.avg_magnitude = sum_mag / agg_data.magnitudes.size();
        } else {
            profile.avg_magnitude = 0.0; // Caso não haja magnitudes válidas
        }

        if (profile.avg_magnitude >= 6.1) {
            profile.score_param2_magnitude = 3; // Alto
        } else if (profile.avg_magnitude >= 5.1) { // 5.1 a 6.0
            profile.score_param2_magnitude = 2; // Médio
        } else { // 0 a 5.0 (inclui o caso de 0.0 por falta de dados válidos)
            profile.score_param2_magnitude = 1; // Baixo
        }


        // Parâmetro 3: Média do Impact Score
        if (!agg_data.impact_scores.empty()) {
            double sum_impact = 0;
            for(double impact : agg_data.impact_scores) sum_impact += impact;
            profile.avg_impact_score = sum_impact / agg_data.impact_scores.size();
        } else {
            profile.avg_impact_score = 0.0; // Caso não haja scores válidos
        }
        
        if (profile.avg_impact_score >= 701) {
            profile.score_param3_impact = 3; // Alto
        } else if (profile.avg_impact_score >= 401) { // 401 a 700
            profile.score_param3_impact = 2; // Médio
        } else { // 0 a 400 (inclui o caso de 0.0 por falta de dados válidos)
            profile.score_param3_impact = 1; // Baixo
        }

        // Cálculo da média final das pontuações dos parâmetros
        profile.final_numeric_score = (static_cast<double>(profile.score_param1_earthquakes) +
                                       profile.score_param2_magnitude +
                                       profile.score_param3_impact) / 3.0;

        // Definição do nível de risco do país
        // 0 a 1: Baixo (<=1.0)
        // 1.1 a 2: Médio (>1.0 e <=2.0)
        // 2.1 a 3: Alto (>2.0)
        if (profile.final_numeric_score > 2.0) { 
            profile.risk_level = "Alto Risco";
        } else if (profile.final_numeric_score > 1.0) { 
            profile.risk_level = "Medio Risco";
        } else { 
            profile.risk_level = "Baixo Risco";
        }
        
        risk_profiles[country_name] = profile;
    }

    return risk_profiles;
}


void print_country_risk_profiles(const std::map<std::string, CountryRiskProfile>& profiles) {
    if (profiles.empty()) {
        std::cout << "Nenhum perfil de risco de pais para exibir." << std::endl;
        return;
    }

    std::cout << "\n--- Classificacao de Risco dos Paises ---" << std::endl;
    std::cout << std::left 
              << std::setw(25) << "Pais"
              << std::setw(12) << "Total EQ"
              << std::setw(12) << "Avg Mag."
              << std::setw(18) << "Avg Impact Scr."
              << std::setw(8) << "Scr EQ"
              << std::setw(8) << "Scr Mag"
              << std::setw(8) << "Scr Imp"
              << std::setw(13) << "Final Score"
              << "Nivel Risco" << std::endl;
    std::cout << std::string(115, '-') << std::endl; // Ajustar o tamanho da linha

    for (const auto& pair : profiles) {
        const CountryRiskProfile& p = pair.second;
        std::cout << std::left 
                  << std::setw(25) << p.country_name
                  << std::right << std::setw(10) << p.total_earthquakes << "  " // Ajuste para alinhar melhor
                  << std::fixed << std::setprecision(2) << std::setw(10) << p.avg_magnitude << "  "
                  << std::fixed << std::setprecision(2) << std::setw(16) << p.avg_impact_score << "  "
                  << std::left // Voltar para left para scores de texto
                  << std::setw(8) << p.score_param1_earthquakes
                  << std::setw(8) << p.score_param2_magnitude
                  << std::setw(8) << p.score_param3_impact
                  << std::fixed << std::setprecision(2) << std::setw(13) << p.final_numeric_score
                  << p.risk_level << std::endl;
    }
    std::cout << std::string(115, '-') << std::endl;
}