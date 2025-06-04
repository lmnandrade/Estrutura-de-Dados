#include "alerta.h"
#include "operations_menu.h" // Se OpsMenuHelpers estiver aqui ou mover helpers para outro lugar
#include <iostream>
#include <iomanip>      // Para std::fixed, std::setprecision
#include <algorithm>    // Para std::transform, std::tolower
#include <stdexcept>    // Para std::stod

// Definição da função
void alerta_terremoto(
    const std::map<std::string, CountryRiskProfile>& country_risks,
    const std::vector<std::string>& registered_emails,
    const std::vector<EarthquakeRecord>& all_raw_records
) {
    std::cout << "\n--- Alerta de Terremoto ---" << std::endl;
    std::string alert_date = OpsMenuHelpers::get_line_input_ops("Data do alerta (AAAA-MM-DD): ");
    std::string alert_time = OpsMenuHelpers::get_line_input_ops("Horario do alerta (UTC HH:MM:SS): ");
    std::string alert_city = OpsMenuHelpers::get_line_input_ops("Cidade do alerta: ");
    std::string alert_country = OpsMenuHelpers::get_line_input_ops("Pais do alerta: ");
    std::string mag_estimada_str = OpsMenuHelpers::get_line_input_ops("Magnitude estimada: ");

    double mag_estimada;
    try {
        mag_estimada = std::stod(mag_estimada_str);
    } catch (const std::exception& e) {
        std::cerr << "ERRO: Magnitude estimada invalida: " << mag_estimada_str << " (" << e.what() << ")" << std::endl;
        return;
    }

    std::cout << "\nAnalisando necessidade de alerta..." << std::endl;
    std::cout << "Evento: " << alert_city << ", " << alert_country << " - Mag. Estimada: " << std::fixed << std::setprecision(1) << mag_estimada << std::endl;

    bool send_alert = false;
    std::string country_risk_level = "Nao Classificado";
    bool country_has_history = false;

    std::string alert_country_clean = alert_country;
    alert_country_clean.erase(0, alert_country_clean.find_first_not_of(" \t\n\r\f\v"));
    alert_country_clean.erase(alert_country_clean.find_last_not_of(" \t\n\r\f\v") + 1);

    for(const auto& rec : all_raw_records){
        std::string rec_country_clean = rec.country;
        rec_country_clean.erase(0, rec_country_clean.find_first_not_of(" \t\n\r\f\v"));
        rec_country_clean.erase(rec_country_clean.find_last_not_of(" \t\n\r\f\v") + 1);
        if(rec_country_clean == alert_country_clean){
            country_has_history = true;
            break;
        }
    }

    auto it_risk = country_risks.find(alert_country_clean);
    if (it_risk != country_risks.end()) {
        country_risk_level = it_risk->second.risk_level;
        std::cout << "Classificacao de risco do pais (" << alert_country_clean << "): " << country_risk_level << std::endl;
    } else {
        std::cout << "Pais (" << alert_country_clean << ") nao encontrado nos perfis de risco atuais." << std::endl;
        if(country_has_history){
            std::cout << "No entanto, o pais possui registros historicos no dataset." << std::endl;
        } else {
            std::cout << "O pais tambem nao possui registros historicos no dataset." << std::endl;
        }
    }

    if (mag_estimada < 3.5) {
        send_alert = false;
        std::cout << "Magnitude < 3.5: Nao representa risco significativo. Alerta nao necessario." << std::endl;
    } else if (mag_estimada >= 3.5 && mag_estimada <= 5.4) {
        std::cout << "Magnitude entre 3.5 e 5.4." << std::endl;
        std::string risk_level_lower = country_risk_level;
        std::transform(risk_level_lower.begin(), risk_level_lower.end(), risk_level_lower.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        if (!country_has_history || risk_level_lower == "baixo risco") {
            send_alert = true;
            if (!country_has_history) std::cout << "Condicao para alerta: Pais sem historico no dataset." << std::endl;
            if (risk_level_lower == "baixo risco") std::cout << "Condicao para alerta: Pais classificado como Baixo Risco." << std::endl;
        } else {
            std::cout << "Condicao para alerta NAO atendida (Pais com historico E nao eh Baixo Risco/Nao Classificado)." << std::endl;
        }
    } else if (mag_estimada >= 5.5) {
        send_alert = true;
        std::cout << "Magnitude >= 5.5: Alerta sera enviado independentemente da classificacao do pais." << std::endl;
    }

    if (send_alert) {
        std::cout << "\nALERTA SERA ENVIADO!" << std::endl;
        std::string alert_scope_message = "Alerta enviado para pessoas em um raio de ate 100km do epicentro.";
        if (mag_estimada > 6.0) {
            alert_scope_message = "Alerta enviado para a CIDADE INTEIRA de " + alert_city + ".";
        }
        std::cout << alert_scope_message << std::endl;

        if (registered_emails.empty()) {
            std::cout << "Nenhum email cadastrado para receber alertas." << std::endl;
        } else {
            std::cout << "Emails que receberiam o alerta:" << std::endl;
            for (const auto& email : registered_emails) {
                std::cout << " - " << email << std::endl;
            }
            std::cout << "\n--- Conteudo do Alerta (Simulacao de Envio) ---" << std::endl;
            std::cout << "Assunto: ALERTA DE TERREMOTO - " << alert_city << ", " << alert_country_clean << std::endl;
            std::cout << "Corpo: Um terremoto com magnitude estimada de " << std::fixed << std::setprecision(1) << mag_estimada
                      << " ocorreu/pode ocorrer proximo a " << alert_city << ", " << alert_country_clean
                      << " aproximadamente em " << alert_date << " as " << alert_time << " (UTC)."
                      << " Por favor, tome as precaucoes necessarias e siga as orientacoes das autoridades locais." << std::endl;
            std::cout << "-----------------------------------------------" << std::endl;
        }
    } else {
        std::cout << "\nAlerta NAO sera enviado com base nos criterios." << std::endl;
    }
}