#include "operations_menu.h"
#include "csv_reader.h" // Para displayRecord e EarthquakeRecord
#include "lista_dupla_ligada.h" // Para DoublyLinkedList, etc.
// Se menu_monitor_terremotos for usar outras estruturas, inclua seus headers aqui.

#include <iostream>
#include <string>
#include <vector>      // Para std::vector
#include <map>         // Para std::map
#include <limits>      // Para std::numeric_limits
#include <iomanip>     // Para formatação
#include <algorithm>   // Para std::transform

// --- Namespace para Funções Auxiliares de Input e Display deste Módulo ---
namespace OpsMenuHelpers {
    void clear_input_buffer_ops() {
        std::cin.clear(); // Limpa flags de erro do cin
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::string get_line_input_ops(const std::string& prompt) {
        std::string input;
        std::cout << prompt;
        std::getline(std::cin, input);
        return input;
    }

    void display_op_search_results(const std::vector<EarthquakeRecord>& results, const std::string& search_type) {
        if (results.empty()) {
            std::cout << "Nenhum registro encontrado para a busca por '" << search_type << "'." << std::endl;
        } else {
            std::cout << "\n--- Resultados da Busca por '" << search_type << "' (" << results.size() << " encontrados) ---" << std::endl;
            int record_num = 1;
            for (const auto& record : results) {
                std::cout << "Resultado #" << record_num++ << ":" << std::endl;
                std::cout << "  "; // Pequeno indent
                displayRecord(record); // Usando a função global displayRecord de csv_reader.h
                // Removida a linha de separador extra daqui para evitar duplicação se displayRecord já tiver uma.
            }
        }
    }
} // namespace OpsMenuHelpers


// --- Implementação do Menu "Monitorar Terremotos" ---
void menu_monitor_terremotos(
    DoublyLinkedList& data_structure, // Usando DLL especificamente aqui, pode ser template depois
    const std::map<std::string, CountryRiskProfile>& country_risks,
    std::vector<EarthquakeRecord>& all_raw_records_ref // Passado por referência para permitir adições
) {
    int choice;
    std::vector<EarthquakeRecord> last_search_results;
    bool can_remove_from_search = false;

    do {
        std::cout << "\n--- Menu Monitorar Terremotos (Usando Lista Duplamente Ligada) ---" << std::endl;
        std::cout << "1. Adicionar Novo Registro" << std::endl;
        std::cout << "2. Remover Registro (Identificacao Manual)" << std::endl;
        std::cout << "3. Buscar Registro" << std::endl;
        std::cout << "4. Listar Registros" << std::endl;
        if (can_remove_from_search && !last_search_results.empty()) {
            std::cout << "5. Remover Registro(s) da Ultima Busca" << std::endl;
        }
        std::cout << "0. Voltar ao Menu Anterior" << std::endl; // Mudado de "Menu Principal" para mais genérico
        std::cout << "Sua escolha: ";
        
        if (!(std::cin >> choice)) {
            std::cout << "ERRO: Entrada invalida. Por favor, insira um numero." << std::endl;
            OpsMenuHelpers::clear_input_buffer_ops();
            choice = -1; // Para continuar no loop
            continue;
        }
        OpsMenuHelpers::clear_input_buffer_ops();
        
        // Resetar a flag can_remove_from_search apenas se a opção de busca não for escolhida
        // ou se a busca não resultar em nada. A busca em si já define essa flag.
        // Aqui, a flag é mais para controlar a exibição da opção 5.
        // A lógica de resetar can_remove_from_search após uma remoção bem sucedida é importante.

        switch (choice) {
            case 1: { // Adicionar
                std::cout << "\n--- Adicionar Novo Registro ---" << std::endl;
                EarthquakeRecord new_record; // Usa EarthquakeRecord de csv_reader.h
                new_record.date = OpsMenuHelpers::get_line_input_ops("Data (AAAA-MM-DD): ");
                new_record.time = OpsMenuHelpers::get_line_input_ops("Hora (UTC HH:MM:SS): ");
                new_record.latitude = OpsMenuHelpers::get_line_input_ops("Latitude: ");
                new_record.longitude = OpsMenuHelpers::get_line_input_ops("Longitude: ");
                new_record.country = OpsMenuHelpers::get_line_input_ops("Pais: ");
                new_record.city = OpsMenuHelpers::get_line_input_ops("Cidade: ");
                new_record.depth = OpsMenuHelpers::get_line_input_ops("Profundidade (km): ");
                new_record.magnitude = OpsMenuHelpers::get_line_input_ops("Magnitude: ");
                new_record.impact_score = OpsMenuHelpers::get_line_input_ops("Impact Score: ");
                
                data_structure.insert_record(new_record);
                all_raw_records_ref.push_back(new_record); 

                std::cout << "Registro adicionado com sucesso." << std::endl;
                std::cout << "NOTA: O perfil de risco dos paises sera recalculado ao sair deste menu." << std::endl;
                break;
            }
            case 2: { // Remover Manual
                std::cout << "\n--- Remover Registro (Identificacao Manual) ---" << std::endl;
                std::string r_date = OpsMenuHelpers::get_line_input_ops("Data (AAAA-MM-DD) do registro a remover: ");
                std::string r_time = OpsMenuHelpers::get_line_input_ops("Hora (UTC HH:MM:SS) do registro a remover: ");
                std::string r_city = OpsMenuHelpers::get_line_input_ops("Cidade do registro a remover: ");
                std::string r_country = OpsMenuHelpers::get_line_input_ops("Pais do registro a remover: ");
                if (data_structure.remove_record(r_date, r_time, r_city, r_country)) {
                    std::cout << "Registro removido da estrutura." << std::endl;
                    // ATENÇÃO: Também precisa remover de 'all_raw_records_ref' para consistência total
                    // na reclassificação, se a reclassificação no main usar all_raw_records_ref como fonte primária.
                    // Se a reclassificação no main SEMPRE usar data_structure.get_all_records_vector(), então
                    // a remoção de all_raw_records_ref não é estritamente necessária aqui, mas pode levar
                    // a inconsistências se all_raw_records_ref for usada para outras coisas.
                    // Para robustez, é melhor remover de ambos.
                    bool removed_from_raw = false;
                    for (auto it_raw = all_raw_records_ref.begin(); it_raw != all_raw_records_ref.end(); ++it_raw) {
                        if (it_raw->date == r_date && it_raw->time == r_time && it_raw->city == r_city && it_raw->country == r_country) {
                            all_raw_records_ref.erase(it_raw);
                            removed_from_raw = true;
                            std::cout << "Registro tambem removido da lista global de registros." << std::endl;
                            break; 
                        }
                    }
                    if(!removed_from_raw){
                        std::cout << "AVISO: Registro removido da estrutura, mas nao encontrado na lista global (pode indicar dessincronizacao)." << std::endl;
                    }

                } else {
                    std::cout << "Registro nao encontrado na estrutura." << std::endl;
                }
                break;
            }
            case 3: { // Buscar
                last_search_results.clear(); 
                can_remove_from_search = false; // Resetar antes de uma nova busca
                int search_choice_monitor;
                std::cout << "\n--- Buscar Registro ---" << std::endl;
                std::cout << "1. Por Cidade" << std::endl;
                std::cout << "2. Por Magnitude (Exata)" << std::endl;
                std::cout << "3. Por Data" << std::endl;
                std::cout << "4. Por Nivel de Risco do Pais" << std::endl;
                std::cout << "5. Por Cidade E Magnitude" << std::endl;
                std::cout << "0. Voltar" << std::endl;
                std::cout << "Escolha o tipo de busca: ";
                
                if (!(std::cin >> search_choice_monitor)) {
                     std::cout << "ERRO: Entrada invalida." << std::endl;
                     OpsMenuHelpers::clear_input_buffer_ops();
                     break;
                }
                OpsMenuHelpers::clear_input_buffer_ops();

                switch (search_choice_monitor) {
                    case 1: {
                        std::string city = OpsMenuHelpers::get_line_input_ops("Digite a cidade: ");
                        last_search_results = data_structure.search_by_city(city);
                        OpsMenuHelpers::display_op_search_results(last_search_results, "Cidade: " + city);
                        break;
                    }
                    case 2: {
                        std::string mag = OpsMenuHelpers::get_line_input_ops("Digite a magnitude exata: ");
                        last_search_results = data_structure.search_by_magnitude_exact(mag);
                        OpsMenuHelpers::display_op_search_results(last_search_results, "Magnitude Exata: " + mag);
                        break;
                    }
                    case 3: {
                        std::string date_s = OpsMenuHelpers::get_line_input_ops("Digite a data (AAAA-MM-DD): ");
                        last_search_results = data_structure.search_by_date(date_s);
                        OpsMenuHelpers::display_op_search_results(last_search_results, "Data: " + date_s);
                        break;
                    }
                    case 4: { // Por Nível de Risco
                        std::string risk_query = OpsMenuHelpers::get_line_input_ops("Nivel de Risco ('Baixo Risco', 'Medio Risco', 'Alto Risco'): ");
                        std::string target_risk_normalized;
                        std::string risk_query_lower = risk_query;
                        std::transform(risk_query_lower.begin(), risk_query_lower.end(), risk_query_lower.begin(), 
                                       [](unsigned char c){ return std::tolower(c); });

                        if (risk_query_lower.find("baixo") != std::string::npos) target_risk_normalized = "baixo risco";
                        else if (risk_query_lower.find("medio") != std::string::npos) target_risk_normalized = "medio risco";
                        else if (risk_query_lower.find("alto") != std::string::npos) target_risk_normalized = "alto risco";
                        else {
                            std::cout << "Nivel de risco desconhecido: '" << risk_query << "'" << std::endl;
                            break;
                        }
                        
                        std::vector<EarthquakeRecord> records_from_current_structure = data_structure.get_all_records_vector();
                        for (const auto& record : records_from_current_structure) {
                            auto it = country_risks.find(record.country);
                            if (it != country_risks.end()) {
                                std::string current_country_risk_level_lower = it->second.risk_level;
                                std::transform(current_country_risk_level_lower.begin(), current_country_risk_level_lower.end(), current_country_risk_level_lower.begin(), 
                                               [](unsigned char c){ return std::tolower(c); });
                                if (current_country_risk_level_lower == target_risk_normalized) {
                                    last_search_results.push_back(record);
                                }
                            }
                        }
                        OpsMenuHelpers::display_op_search_results(last_search_results, "Nivel de Risco: " + risk_query);
                        break;
                    }
                    case 5: {
                        std::string city_cm = OpsMenuHelpers::get_line_input_ops("Digite a cidade: ");
                        std::string mag_cm = OpsMenuHelpers::get_line_input_ops("Digite a magnitude: ");
                        last_search_results = data_structure.search_by_city_and_magnitude(city_cm, mag_cm);
                        OpsMenuHelpers::display_op_search_results(last_search_results, "Cidade: " + city_cm + " e Mag: " + mag_cm);
                        break;
                    }
                    case 0: break;
                    default: std::cout << "Opcao de busca invalida." << std::endl;
                }
                if (!last_search_results.empty()) {
                    can_remove_from_search = true;
                }
                break;
            }
            case 4: { // Listar
                int list_choice_monitor;
                std::cout << "\n--- Listar Registros ---" << std::endl;
                std::cout << "1. Listar Todos" << std::endl;
                std::cout << "2. Listar por Magnitude Minima" << std::endl;
                std::cout << "3. Listar por Ano" << std::endl;
                std::cout << "0. Voltar" << std::endl;
                std::cout << "Escolha: ";
                if (!(std::cin >> list_choice_monitor)) {
                    std::cout << "ERRO: Entrada invalida." << std::endl;
                    OpsMenuHelpers::clear_input_buffer_ops();
                    break;
                }
                OpsMenuHelpers::clear_input_buffer_ops();

                switch (list_choice_monitor) {
                    case 1: data_structure.list_all_records(); break;
                    case 2: {
                        std::string min_mag_l = OpsMenuHelpers::get_line_input_ops("Digite a magnitude minima: ");
                        data_structure.list_by_min_magnitude(min_mag_l);
                        break;
                    }
                    case 3: {
                        std::string year_l = OpsMenuHelpers::get_line_input_ops("Digite o ano (AAAA): ");
                        data_structure.list_by_year(year_l);
                        break;
                    }
                    case 0: break;
                    default: std::cout << "Opcao de listagem invalida." << std::endl;
                }
                break;
            }
            case 5: { // Remover da última busca
                if (can_remove_from_search && !last_search_results.empty()) {
                    std::cout << "\n--- Remover Registros da Ultima Busca ---" << std::endl;
                    // Para remoção interativa, pode ser melhor listar os resultados com um índice
                    // e pedir ao usuário qual índice remover.
                    // Por simplicidade, vamos remover todos os resultados da última busca.
                    std::cout << last_search_results.size() << " registro(s) encontrado(s) na ultima busca. Remover todos?" << std::endl;
                    std::cout << "Digite 's' para confirmar ou qualquer outra tecla para cancelar: ";
                    std::string confirm_remove = OpsMenuHelpers::get_line_input_ops("");
                    if (confirm_remove == "s" || confirm_remove == "S") {
                        int removed_count = 0;
                        for (const auto& rec_to_remove : last_search_results) {
                            if (data_structure.remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city, rec_to_remove.country)) {
                                removed_count++;
                                // Também remover de all_raw_records_ref
                                for (auto it_raw = all_raw_records_ref.begin(); it_raw != all_raw_records_ref.end(); ) {
                                    if (it_raw->date == rec_to_remove.date && it_raw->time == rec_to_remove.time && 
                                        it_raw->city == rec_to_remove.city && it_raw->country == rec_to_remove.country) {
                                        it_raw = all_raw_records_ref.erase(it_raw); // erase retorna o próximo iterador
                                    } else {
                                        ++it_raw;
                                    }
                                }
                            }
                        }
                        std::cout << removed_count << " registro(s) removido(s)." << std::endl;
                    } else {
                        std::cout << "Remocao cancelada." << std::endl;
                    }
                    last_search_results.clear();
                    can_remove_from_search = false; // Desabilitar opção após tentativa de remoção
                } else {
                     if (choice == 5) { // Se o usuário digitou 5 diretamente sem busca prévia
                        std::cout << "Opcao 5 (Remover da Ultima Busca) so esta disponivel apos uma busca bem sucedida." << std::endl;
                     } else { // Isso não deveria acontecer se o menu só mostra a opção 5 quando `can_remove_from_search` é true
                        std::cout << "Nao ha resultados de busca anteriores para remover ou opcao nao disponivel." << std::endl;
                     }
                }
                break;
            }
            case 0:
                std::cout << "Voltando ao menu anterior..." << std::endl;
                break;
            default:
                std::cout << "Opcao invalida. Tente novamente." << std::endl;
        }
    } while (choice != 0);
}


// --- Implementação do Menu "Alerta de Terremoto" ---
void menu_alerta_terremoto(
    const std::map<std::string, CountryRiskProfile>& country_risks,
    const std::vector<std::string>& registered_emails,
    const std::vector<EarthquakeRecord>& all_raw_records // Usado para verificar histórico do país
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

    std::string alert_country_clean = alert_country; // Limpar nome do país para busca no mapa
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
            // Simulação do envio de email
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