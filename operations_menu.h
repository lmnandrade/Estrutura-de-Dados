#ifndef OPERATIONS_MENU_H
#define OPERATIONS_MENU_H

#include "csv_reader.h"
#include "classification.h"
// Incluir headers das estruturas
#include "lista_dupla_ligada.h"
#include "avl_tree.h"
#include "kd_tree.h"

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <type_traits> // Para std::is_same

// --- Namespace para Funções Auxiliares de Input ---
namespace OpsMenuHelpers {
    inline void clear_input_buffer_ops() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    inline std::string get_line_input_ops(const std::string& prompt) {
        std::string input;
        std::cout << prompt;
        std::getline(std::cin, input);
        return input;
    }

     inline void display_op_search_results(const std::vector<EarthquakeRecord>& results, const std::string& search_type) {
        if (results.empty()) {
            std::cout << "Nenhum registro encontrado para a busca por '" << search_type << "'." << std::endl;
        } else {
            std::cout << "\n--- Resultados da Busca por '" << search_type << "' (" << results.size() << " encontrados) ---" << std::endl;
            int record_num = 1;
            for (const auto& record : results) {
                std::cout << "Resultado #" << record_num++ << ":" << std::endl;
                std::cout << "  ";
                displayRecord(record); // Função global de csv_reader.h
            }
            std::cout << "--------------------------------------------------------" << std::endl;
        }
    }
} // Fim do namespace OpsMenuHelpers


// --- Funções Auxiliares para Abstrair a Chamada get_all_records ---
inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const DoublyLinkedList& ds) {
    return ds.get_all_records_vector();
}

inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const AVLTree& ds) {
    return ds.get_all_records();
}

inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const KDTree& ds) {
    return ds.get_all_records_vector();
}


template <typename DS>
std::vector<EarthquakeRecord> get_all_records_from_structure(const DS& data_structure) {
    return get_all_records_from_structure_impl(data_structure);
}

// --- Definição da Função Template "menu_monitor_terremotos" ---
// A definição completa de uma função template precisa estar no header.
template <typename DataStructureType>
void menu_monitor_terremotos(
    DataStructureType& data_structure, // Não const, pois modificamos
    const std::string& structure_name,
    const std::map<std::string, CountryRiskProfile>& country_risks, // const ref
    std::vector<EarthquakeRecord>& all_raw_records_ref // ref para modificar lista global
) {
    int choice;
    std::vector<EarthquakeRecord> last_search_results;
    bool can_remove_from_search = false;

    do {
        std::cout << "\n--- Menu Monitorar Terremotos (Usando " << structure_name << ") ---" << std::endl;
        std::cout << "1. Adicionar Novo Registro" << std::endl;
        std::cout << "2. Remover Registro (Identificacao Manual)" << std::endl;
        std::cout << "3. Buscar Registro (Por Atributos)" << std::endl;
        std::cout << "4. Listar Registros" << std::endl;

        // Opções específicas para KD-Tree
        if (std::is_same<DataStructureType, KDTree>::value) {
            std::cout << "6. Buscar por Range Geografico (KD-Tree)" << std::endl;
            std::cout << "7. Buscar Vizinho Mais Proximo (KD-Tree)" << std::endl;
            std::cout << "8. Buscar K Vizinhos Mais Proximos (KD-Tree)" << std::endl;
        }

        if (can_remove_from_search && !last_search_results.empty()) {
            std::cout << "5. Remover Registro(s) da Ultima Busca" << std::endl;
        }
        std::cout << "0. Voltar ao Menu Anterior" << std::endl;
        std::cout << "Sua escolha: ";

        if (!(std::cin >> choice)) {
            std::cout << "ERRO: Entrada invalida. Por favor, insira um numero." << std::endl;
            OpsMenuHelpers::clear_input_buffer_ops();
            choice = -1; // Para continuar no loop
            continue;
        }
        OpsMenuHelpers::clear_input_buffer_ops();

        switch (choice) {
            case 1: { // Adicionar Novo Registro
                std::cout << "\n--- Adicionar Novo Registro ---" << std::endl;
                EarthquakeRecord new_record;
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
                std::cout << "NOTA: O perfil de risco dos paises sera recalculado ao sair deste menu (se houver alteracoes)." << std::endl;
                break;
            }
            case 2: { // Remover Registro (Identificacao Manual)
                std::cout << "\n--- Remover Registro (Identificacao Manual) ---" << std::endl;
                std::string r_date = OpsMenuHelpers::get_line_input_ops("Data (AAAA-MM-DD) do registro a remover: ");
                std::string r_time = OpsMenuHelpers::get_line_input_ops("Hora (UTC HH:MM:SS) do registro a remover: ");
                std::string r_city = OpsMenuHelpers::get_line_input_ops("Cidade do registro a remover: ");
                std::string r_country = OpsMenuHelpers::get_line_input_ops("Pais do registro a remover: ");
                
                if (data_structure.remove_record(r_date, r_time, r_city, r_country)) {
                    std::cout << "Registro removido da estrutura '" << structure_name << "'." << std::endl;
                    bool removed_from_raw = false;
                    for (auto it_raw = all_raw_records_ref.begin(); it_raw != all_raw_records_ref.end(); ) {
                        if (it_raw->date == r_date && it_raw->time == r_time && it_raw->city == r_city && it_raw->country == r_country) {
                            it_raw = all_raw_records_ref.erase(it_raw);
                            removed_from_raw = true;
                            std::cout << "Registro tambem removido da lista global de registros." << std::endl;
                            break; 
                        } else {
                            ++it_raw;
                        }
                    }
                    if(!removed_from_raw && !all_raw_records_ref.empty()){
                        std::cout << "AVISO: Registro removido da estrutura, mas nao encontrado/removido da lista global com a chave exata." << std::endl;
                    }
                } else {
                    std::cout << "Registro nao encontrado na estrutura '" << structure_name << "'." << std::endl;
                }
                break;
            }
            case 3: { // Buscar Registro (Por Atributos)
                last_search_results.clear(); 
                can_remove_from_search = false;
                int search_choice_monitor;
                std::cout << "\n--- Buscar Registro (Por Atributos) ---" << std::endl;
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
                    case 4: { 
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
                        
                        std::vector<EarthquakeRecord> records_from_current_structure = get_all_records_from_structure(data_structure);
                        for (const auto& record : records_from_current_structure) {
                            std::string country_key_record = record.country;
                            country_key_record.erase(0, country_key_record.find_first_not_of(" \t\n\r\f\v"));
                            country_key_record.erase(country_key_record.find_last_not_of(" \t\n\r\f\v") + 1);

                            auto it = country_risks.find(country_key_record); 
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
            case 4: { // Listar Registros
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
            case 5: { // Remover da Última Busca
                if (can_remove_from_search && !last_search_results.empty()) {
                    std::cout << "\n--- Remover Registros da Ultima Busca ---" << std::endl;
                    std::cout << last_search_results.size() << " registro(s) encontrado(s) na ultima busca. Remover todos?" << std::endl;
                    std::cout << "Digite 's' para confirmar ou qualquer outra tecla para cancelar: ";
                    std::string confirm_remove = OpsMenuHelpers::get_line_input_ops("");
                    if (confirm_remove == "s" || confirm_remove == "S") {
                        int removed_count_ds = 0;
                        int removed_count_raw = 0;
                        for (const auto& rec_to_remove : last_search_results) {
                            if (data_structure.remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city, rec_to_remove.country)) {
                                removed_count_ds++;
                            }
                            for (auto it_raw = all_raw_records_ref.begin(); it_raw != all_raw_records_ref.end(); ) {
                                if (it_raw->date == rec_to_remove.date && it_raw->time == rec_to_remove.time &&
                                    it_raw->city == rec_to_remove.city && it_raw->country == rec_to_remove.country) {
                                    it_raw = all_raw_records_ref.erase(it_raw);
                                    removed_count_raw++;
                                } else {
                                    ++it_raw;
                                }
                            }
                        }
                        std::cout << removed_count_ds << " registro(s) removido(s) da estrutura '" << structure_name << "'." << std::endl;
                        std::cout << removed_count_raw << " registro(s) removido(s) da lista global." << std::endl;
                    } else {
                        std::cout << "Remocao cancelada." << std::endl;
                    }
                    last_search_results.clear();
                    can_remove_from_search = false;
                } else if (choice == 5) {
                    std::cout << "Opcao 5 (Remover da Ultima Busca) so esta disponivel apos uma busca bem sucedida com resultados." << std::endl;
                }
                break;
            }
            case 6: { 
                if (std::is_same<DataStructureType, KDTree>::value) {
                    auto& kd_ref = reinterpret_cast<KDTree&>(data_structure);
                    std::cout << "\n--- Buscar por Range Geografico (KD-Tree) ---" << std::endl;
                    double min_lat_val, max_lat_val, min_lon_val, max_lon_val;
                    std::cout << "Digite a Latitude Minima: "; if(!(std::cin >> min_lat_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    std::cout << "Digite a Latitude Maxima: "; if(!(std::cin >> max_lat_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    std::cout << "Digite a Longitude Minima: "; if(!(std::cin >> min_lon_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    std::cout << "Digite a Longitude Maxima: "; if(!(std::cin >> max_lon_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    
                    last_search_results = kd_ref.search_by_range(min_lat_val, max_lat_val, min_lon_val, max_lon_val);
                    OpsMenuHelpers::display_op_search_results(last_search_results, 
                        "Range: Lat[" + std::to_string(min_lat_val) + "," + std::to_string(max_lat_val) + 
                        "] Lon[" + std::to_string(min_lon_val) + "," + std::to_string(max_lon_val) + "]");
                    if (!last_search_results.empty()) can_remove_from_search = true; else can_remove_from_search = false;
                } else if (choice == 6) { 
                     std::cout << "Opcao 6 so esta disponivel para a KD-Tree." << std::endl;
                }
                break;
            }
            case 7: { 
                if (std::is_same<DataStructureType, KDTree>::value) {
                    auto& kd_ref = reinterpret_cast<KDTree&>(data_structure);
                    std::cout << "\n--- Buscar Vizinho Mais Proximo (KD-Tree) ---" << std::endl;
                    double q_lat_val, q_lon_val;
                    std::cout << "Digite a Latitude do Ponto de Consulta: "; if(!(std::cin >> q_lat_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    std::cout << "Digite a Longitude do Ponto de Consulta: "; if(!(std::cin >> q_lon_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();

                    EarthquakeRecord* nearest = kd_ref.search_nearest_neighbor(q_lat_val, q_lon_val);
                    last_search_results.clear(); 
                    if (nearest) {
                        last_search_results.push_back(*nearest); 
                        OpsMenuHelpers::display_op_search_results(last_search_results, 
                            "Vizinho Mais Proximo de (" + std::to_string(q_lat_val) + "," + std::to_string(q_lon_val) + ")");
                        can_remove_from_search = true;
                    } else {
                        std::cout << "Nenhum vizinho encontrado." << std::endl;
                        can_remove_from_search = false;
                    }
                } else if (choice == 7) {
                    std::cout << "Opcao 7 so esta disponivel para a KD-Tree." << std::endl;
                }
                break; 
            }
            case 8: { 
                if (std::is_same<DataStructureType, KDTree>::value) {
                    auto& kd_ref = reinterpret_cast<KDTree&>(data_structure);
                    std::cout << "\n--- Buscar K Vizinhos Mais Proximos (KD-Tree) ---" << std::endl;
                    double q_lat_k_val, q_lon_k_val;
                    unsigned k_val_val;
                    std::cout << "Digite a Latitude do Ponto de Consulta: "; if(!(std::cin >> q_lat_k_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    std::cout << "Digite a Longitude do Ponto de Consulta: "; if(!(std::cin >> q_lon_k_val)) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida.\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    std::cout << "Digite o valor de K: "; if(!(std::cin >> k_val_val) || k_val_val == 0) { OpsMenuHelpers::clear_input_buffer_ops(); std::cout << "Entrada invalida para K (deve ser > 0).\n"; break;} OpsMenuHelpers::clear_input_buffer_ops();
                    
                    last_search_results = kd_ref.search_k_nearest_neighbors(q_lat_k_val, q_lon_k_val, k_val_val);
                    OpsMenuHelpers::display_op_search_results(last_search_results, 
                        std::to_string(k_val_val) + " Vizinhos Mais Proximos de (" + std::to_string(q_lat_k_val) + "," + std::to_string(q_lon_k_val) + ")");
                    if (!last_search_results.empty()) can_remove_from_search = true; else can_remove_from_search = false;
                } else if (choice == 8) {
                    std::cout << "Opcao 8 so esta disponivel para a KD-Tree." << std::endl;
                }
                break;
            }
            case 0:
                std::cout << "Voltando ao menu anterior..." << std::endl;
                break;
            default:
                if ((choice == 6 || choice == 7 || choice == 8) && !std::is_same<DataStructureType, KDTree>::value) {
                    std::cout << "Opcao " << choice << " so esta disponivel para a KD-Tree." << std::endl;
                } else {
                    std::cout << "Opcao invalida. Tente novamente." << std::endl;
                }
                break; 
        }
    } while (choice != 0);
}

#endif // OPERATIONS_MENU_H