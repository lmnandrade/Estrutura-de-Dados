#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

#include "csv_reader.h" // Para EarthquakeRecord
#include <string>
#include <vector>
#include <map>


// Estrutura para armazenar os detalhes da classificação de um país
struct CountryRiskProfile {
    std::string country_name;
    int total_earthquakes;
    double avg_magnitude;
    double avg_impact_score;

    int score_param1_earthquakes; // Pontuação para o número total de terremotos
    int score_param2_magnitude;   // Pontuação para a média das magnitudes
    int score_param3_impact;      // Pontuação para a média do Impact Score

    double final_numeric_score;   // Média das três pontuações acima
    std::string risk_level;       // "Baixo Risco", "Medio Risco", "Alto Risco"

    // Construtor padrão para inicializar
    CountryRiskProfile() :
        total_earthquakes(0),
        avg_magnitude(0.0),
        avg_impact_score(0.0),
        score_param1_earthquakes(0),
        score_param2_magnitude(0),
        score_param3_impact(0),
        final_numeric_score(0.0) {}
};

// Função principal para classificar os países
// Recebe todos os registros de terremotos lidos do CSV
// Retorna um mapa onde a chave é o nome do país e o valor é seu perfil de risco
std::map<std::string, CountryRiskProfile> classify_countries_by_risk(
    const std::vector<EarthquakeRecord>& all_records
);

// Função auxiliar para exibir os resultados da classificação
void print_country_risk_profiles(const std::map<std::string, CountryRiskProfile>& profiles);

#endif // CLASSIFICATION_H