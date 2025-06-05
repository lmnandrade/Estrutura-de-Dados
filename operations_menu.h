#ifndef OPERATIONS_MENU_H
#define OPERATIONS_MENU_H

#include "csv_reader.h"
#include "classification.h"
#include "lista_dupla_ligada.h"
#include "avl_tree.h"
#include "kd_tree.h"
#include "hash_table.h"
#include "skip_list.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <type_traits>

// --- Namespace para Funções Auxiliares KDTree ---
namespace KDTreeHelpers {
    inline bool try_string_to_double(const std::string& str, double& out, bool show_error_msg = false, const std::string& field_name = "") {
        try {
            size_t idx;
            out = std::stod(str, &idx);
            if (str.empty() && idx == 0 && !str.empty()) return false; // Se str for vazia, stod pode ter idx=0 e não lançar exceção
            if (str.empty() && idx==0) { // Para string vazia que não lança exceção
                 if(show_error_msg) std::cerr << "ERRO: Entrada vazia para conversao para double em '" << (field_name.empty() ? "campo desconhecido" : field_name) << "'."<< std::endl;
                return false;
            }
            for (size_t i = idx; i < str.length(); ++i) {
                if (!std::isspace(static_cast<unsigned char>(str[i]))) {
                    if(show_error_msg) std::cerr << "ERRO: Caracteres extras apos o numero em '" << (field_name.empty() ? str : field_name) << "': " << str.substr(idx) << std::endl;
                    return false;
                }
            }
            return true;
        } catch (const std::invalid_argument& ia) {
            if(show_error_msg) std::cerr << "ERRO: Argumento invalido para conversao para double em '" << (field_name.empty() ? str : field_name) << "': " << ia.what() << std::endl;
            return false;
        } catch (const std::out_of_range& oor) {
             if(show_error_msg) std::cerr << "ERRO: Valor fora do intervalo para conversao para double em '" << (field_name.empty() ? str : field_name) << "': " << oor.what() << std::endl;
            return false;
        }
    }
} // Fim do namespace KDTreeHelpers

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
        // Trim whitespace from both ends
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
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
                displayRecord(record); // Assume displayRecord está definida globalmente ou em csv_reader.h
                if (results.size() > 1 && static_cast<size_t>(record_num) <= results.size()){ // Evita linha extra no último
                    std::cout << "  --------------------------------------------------------------------------------" << std::endl;
                }
            }
             if (!results.empty()) { // Linha final se houver resultados
                std::cout << "--------------------------------------------------------------------------------" << std::endl;
            }
        }
    }
} // Fim do namespace OpsMenuHelpers

// --- Funções Auxiliares para Abstrair a Chamada get_all_records ---
// (Mantidas como no original)
inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const DoublyLinkedList& ds) {
    return ds.get_all_records_vector();
}
inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const AVLTree& ds) {
    return ds.get_all_records();
}
inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const KDTree& ds) {
    return ds.get_all_records_vector();
}
inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const HashTable& ds) {
    return ds.get_all_records_vector();
}
inline std::vector<EarthquakeRecord> get_all_records_from_structure_impl(const SkipList& ds) {
    return ds.get_all_records_vector();
}
template <typename DS>
std::vector<EarthquakeRecord> get_all_records_from_structure(const DS& data_structure) {
    return get_all_records_from_structure_impl(data_structure);
}

// --- Menu Principal ---
template <typename DataStructureType>
void menu_monitor_terremotos(
    DataStructureType& data_structure,
    const std::string& structure_name,
    const std::map<std::string, CountryRiskProfile>& country_risks,
    std::vector<EarthquakeRecord>& all_raw_records_ref
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

        if (can_remove_from_search && !last_search_results.empty()) {
            std::cout << "5. Remover Registro(s) da Ultima Busca" << std::endl;
        }

        if (std::is_same<DataStructureType, KDTree>::value) {
            std::cout << "6. Buscar por Range Geografico (KD-Tree)" << std::endl;
            std::cout << "7. Buscar Vizinho Mais Proximo (KD-Tree)" << std::endl;
            std::cout << "8. Buscar K Vizinhos Mais Proximos (KD-Tree)" << std::endl;
        } else if (std::is_same<DataStructureType, HashTable>::value) {
            std::cout << "6. Ver Distribuicao da Tabela Hash" << std::endl;
        }

        std::cout << "0. Voltar ao Menu Anterior" << std::endl;
        std::cout << "Sua escolha: ";

        if (!(std::cin >> choice)) {
            std::cout << "ERRO: Entrada invalida. Por favor, insira um numero." << std::endl;
            OpsMenuHelpers::clear_input_buffer_ops();
            choice = -1;
            continue;
        }
        OpsMenuHelpers::clear_input_buffer_ops();

        if (choice != 5) { // Se não for a opção 5, ou se a opção 5 falhar/cancelar,
                            // o flag para remover da busca deve ser resetado.
            can_remove_from_search = false;
        }
        // Se uma busca (case 3, 6, 7, 8) resultar em last_search_results vazio,
        // can_remove_from_search também deve ser false. Isso é tratado ao final dessas buscas.


        switch (choice) {
            case 1: { // Adicionar Novo Registro
                // (Código original do case 1 mantido)
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
                std::string impact_score_str = OpsMenuHelpers::get_line_input_ops("Impact Score (opcional, deixe em branco se nao souber): ");
                new_record.impact_score = impact_score_str.empty() ? "0" : impact_score_str;

                bool can_insert = true;
                if (std::is_same<DataStructureType, KDTree>::value) {
                    double temp_lat, temp_lon;
                    if (!KDTreeHelpers::try_string_to_double(new_record.latitude, temp_lat, true, "Latitude") ||
                        !KDTreeHelpers::try_string_to_double(new_record.longitude, temp_lon, true, "Longitude")) {
                        std::cout << "AVISO: Latitude ou Longitude invalidas. Registro NAO adicionado a KD-Tree." << std::endl;
                        can_insert = false;
                    }
                }

                if(can_insert) {
                    data_structure.insert_record(new_record);
                    all_raw_records_ref.push_back(new_record); // Adiciona à lista global
                    std::cout << "Registro adicionado com sucesso a '" << structure_name << "' e a lista global." << std::endl;
                }
                break;
            }
            case 2: { // Remover Registro (Identificacao Manual)
                // (Código original do case 2 mantido)
                std::cout << "\n--- Remover Registro (Identificacao Manual) ---" << std::endl;
                std::string r_date = OpsMenuHelpers::get_line_input_ops("Data (AAAA-MM-DD) do registro a remover: ");
                std::string r_time = OpsMenuHelpers::get_line_input_ops("Hora (UTC HH:MM:SS) do registro a remover: ");
                std::string r_city = OpsMenuHelpers::get_line_input_ops("Cidade do registro a remover: ");
                std::string r_country;

                bool removed_from_ds = false;

                if (std::is_same<DataStructureType, HashTable>::value) {
                    auto& ht_ref = reinterpret_cast<HashTable&>(data_structure);
                    removed_from_ds = ht_ref.remove_record(r_date, r_time, r_city);
                    if (removed_from_ds) {
                         r_country = OpsMenuHelpers::get_line_input_ops("Pais do registro (para confirmar remocao da lista global): ");
                    }
                } else {
                    r_country = OpsMenuHelpers::get_line_input_ops("Pais do registro a remover: ");
                    if (std::is_same<DataStructureType, DoublyLinkedList>::value) {
                        removed_from_ds = reinterpret_cast<DoublyLinkedList&>(data_structure).remove_record(r_date, r_time, r_city, r_country);
                    } else if (std::is_same<DataStructureType, AVLTree>::value) {
                        removed_from_ds = reinterpret_cast<AVLTree&>(data_structure).remove_record(r_date, r_time, r_city, r_country);
                    } else if (std::is_same<DataStructureType, KDTree>::value) {
                        removed_from_ds = reinterpret_cast<KDTree&>(data_structure).remove_record(r_date, r_time, r_city, r_country);
                    } else if (std::is_same<DataStructureType, SkipList>::value) {
                        removed_from_ds = reinterpret_cast<SkipList&>(data_structure).remove_record(r_date, r_time, r_city, r_country);
                    }
                }

                if (removed_from_ds) {
                    std::cout << "Registro removido da estrutura '" << structure_name << "'." << std::endl;
                    bool removed_from_raw = false;
                    if (r_country.empty() && removed_from_ds && std::is_same<DataStructureType, HashTable>::value ) { // Apenas para HT que pode não ter o país inicialmente
                        std::cout << "AVISO: Pais nao fornecido/necessario para remocao da Tabela Hash. A tentativa de remocao da lista global pode ser imprecisa ou pedir confirmacao." << std::endl;
                    }

                    for (auto it_raw = all_raw_records_ref.begin(); it_raw != all_raw_records_ref.end(); ) {
                        bool match = (it_raw->date == r_date && it_raw->time == r_time && it_raw->city == r_city);
                        if (match && !r_country.empty()) {
                            match = (it_raw->country == r_country);
                        }

                        if (match) {
                            if (r_country.empty() && std::is_same<DataStructureType, HashTable>::value && removed_from_ds) {
                                std::cout << "Registro encontrado na lista global (data/hora/cidade correspondem, pais nao verificado):" << std::endl;
                                std::cout << "  "; displayRecord(*it_raw);
                                std::string confirm_raw_remove = OpsMenuHelpers::get_line_input_ops("Confirmar remocao deste registro da lista global? (s/n): ");
                                if (confirm_raw_remove != "s" && confirm_raw_remove != "S") {
                                    std::cout << "Remocao da lista global cancelada para este registro." << std::endl;
                                    ++it_raw;
                                    continue;
                                }
                            }
                            it_raw = all_raw_records_ref.erase(it_raw);
                            removed_from_raw = true;
                            std::cout << "Registro tambem removido da lista global de registros." << std::endl;
                            break;
                        } else {
                            ++it_raw;
                        }
                    }
                    if(!removed_from_raw && removed_from_ds){ // Apenas se removeu da DS e não da raw
                        std::cout << "AVISO: Registro removido da estrutura, mas nao encontrado/removido da lista global com os dados fornecidos." << std::endl;
                    }
                } else {
                    std::cout << "Registro nao encontrado na estrutura '" << structure_name << "' com os identificadores fornecidos." << std::endl;
                }
                break;
            }
            case 3: { // Buscar Registro (Por Atributos)
                // (Código original do case 3 mantido, incluindo can_remove_from_search = true; no final se resultados)
                last_search_results.clear();
                int search_choice_monitor;
                std::cout << "\n--- Buscar Registro (Por Atributos) em " << structure_name << " ---" << std::endl;
                std::cout << "1. Por Cidade" << std::endl;
                std::cout << "2. Por Magnitude (Exata)" << std::endl;
                std::cout << "3. Por Data (AAAA-MM-DD)" << std::endl;
                std::cout << "4. Por Nivel de Risco do Pais" << std::endl;
                std::cout << "5. Por Cidade E Magnitude" << std::endl;
                if (std::is_same<DataStructureType, HashTable>::value || std::is_same<DataStructureType, SkipList>::value) {
                    std::cout << "6. Por Chave Primaria Exata (Data_Hora_Cidade)" << std::endl;
                }
                std::cout << "0. Voltar" << std::endl;
                std::cout << "Escolha o tipo de busca: ";

                if (!(std::cin >> search_choice_monitor)) {
                     std::cout << "ERRO: Entrada invalida." << std::endl;
                     OpsMenuHelpers::clear_input_buffer_ops();
                     break;
                }
                OpsMenuHelpers::clear_input_buffer_ops();

                if ((std::is_same<DataStructureType, HashTable>::value || std::is_same<DataStructureType, SkipList>::value) && search_choice_monitor == 6) {
                    std::string date_key = OpsMenuHelpers::get_line_input_ops("Data (AAAA-MM-DD) da chave: ");
                    std::string time_key = OpsMenuHelpers::get_line_input_ops("Hora (UTC HH:MM:SS) da chave: ");
                    std::string city_key = OpsMenuHelpers::get_line_input_ops("Cidade da chave: ");
                    EarthquakeRecord* found_rec = nullptr;
                    if (std::is_same<DataStructureType, HashTable>::value) {
                        auto& ht_ref = reinterpret_cast<HashTable&>(data_structure);
                        found_rec = const_cast<EarthquakeRecord*>(ht_ref.search_record(date_key, time_key, city_key));
                    } else { 
                        auto& sl_ref = reinterpret_cast<SkipList&>(data_structure);
                        found_rec = const_cast<EarthquakeRecord*>(sl_ref.search_record(date_key, time_key, city_key));
                    }
                    if (found_rec) {
                        last_search_results.push_back(*found_rec);
                    }
                    OpsMenuHelpers::display_op_search_results(last_search_results, "Chave Primaria Exata: " + date_key + "_" + time_key + "_" + city_key);
                } else {
                    std::vector<EarthquakeRecord> records_for_search = get_all_records_from_structure(data_structure);
                    std::string search_description;
                    bool valid_search_option = true;

                    switch (search_choice_monitor) {
                        case 1: {
                            std::string city_s = OpsMenuHelpers::get_line_input_ops("Digite a cidade: ");
                            search_description = "Cidade: " + city_s;
                            for(const auto& rec : records_for_search) if(rec.city == city_s) last_search_results.push_back(rec);
                            break;
                        }
                        case 2: {
                            std::string mag = OpsMenuHelpers::get_line_input_ops("Digite a magnitude exata: ");
                            search_description = "Magnitude Exata: " + mag;
                            for(const auto& rec : records_for_search) if(rec.magnitude == mag) last_search_results.push_back(rec);
                            break;
                        }
                        case 3: {
                            std::string date_s = OpsMenuHelpers::get_line_input_ops("Digite a data (AAAA-MM-DD): ");
                            search_description = "Data: " + date_s;
                            for(const auto& rec : records_for_search) if(rec.date == date_s) last_search_results.push_back(rec);
                            break;
                        }
                        case 4: {
                            std::string risk_query = OpsMenuHelpers::get_line_input_ops("Nivel de Risco ('Baixo Risco', 'Medio Risco', 'Alto Risco'): ");
                            search_description = "Nivel de Risco: " + risk_query;
                            std::string target_risk_normalized;
                            std::string risk_query_lower = risk_query;
                            std::transform(risk_query_lower.begin(), risk_query_lower.end(), risk_query_lower.begin(), ::tolower);
                            if (risk_query_lower.find("baixo") != std::string::npos) target_risk_normalized = "baixo risco";
                            else if (risk_query_lower.find("medio") != std::string::npos) target_risk_normalized = "medio risco";
                            else if (risk_query_lower.find("alto") != std::string::npos) target_risk_normalized = "alto risco";
                            else { std::cout << "Nivel de risco desconhecido: '" << risk_query << "'.\n"; valid_search_option = false; break; }

                            for (const auto& record : records_for_search) {
                                std::string country_key_record = record.country;
                                country_key_record.erase(0, country_key_record.find_first_not_of(" \t\n\r\f\v"));
                                country_key_record.erase(country_key_record.find_last_not_of(" \t\n\r\f\v") + 1);
                                auto it = country_risks.find(country_key_record);
                                if (it != country_risks.end()) {
                                    std::string current_country_risk = it->second.risk_level;
                                    std::transform(current_country_risk.begin(), current_country_risk.end(), current_country_risk.begin(), ::tolower);
                                    if (current_country_risk == target_risk_normalized) last_search_results.push_back(record);
                                }
                            }
                            break;
                        }
                        case 5: {
                            std::string city_cm = OpsMenuHelpers::get_line_input_ops("Digite a cidade: ");
                            std::string mag_cm = OpsMenuHelpers::get_line_input_ops("Digite a magnitude: ");
                            search_description = "Cidade: " + city_cm + " e Mag: " + mag_cm;
                            for(const auto& rec : records_for_search) if(rec.city == city_cm && rec.magnitude == mag_cm) last_search_results.push_back(rec);
                            break;
                        }
                        case 0: valid_search_option = false; break;
                        default:
                            valid_search_option = false;
                            if (search_choice_monitor == 6 && !(std::is_same<DataStructureType, HashTable>::value || std::is_same<DataStructureType, SkipList>::value)) {
                                std::cout << "Opcao 6 (Busca por Chave Primaria) so esta disponivel para Tabela Hash ou Skip List." << std::endl;
                            } else {
                                std::cout << "Opcao de busca invalida." << std::endl;
                            }
                    }
                    if (valid_search_option && search_choice_monitor != 0) {
                        OpsMenuHelpers::display_op_search_results(last_search_results, search_description);
                    }
                }
                if (!last_search_results.empty()) {
                    can_remove_from_search = true;
                } else {
                    can_remove_from_search = false; // Garante que se a busca não tiver resultados, a opção 5 não apareça
                }
                break;
            }
            case 4: { // Listar Registros
                // (Código original do case 4 mantido)
                // Esta seção assume que data_structure TEM os métodos list_all_records, etc.
                // Se não, causará erro de compilação para essa DataStructureType.
                // "Consertar" isso sem alterar a função significaria que as estruturas DEVEM ter esses métodos.
                int list_choice_monitor;
                std::cout << "\n--- Listar Registros em " << structure_name << " ---" << std::endl;
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
                    std::cout << "\n--- Remover Registros da Ultima Busca (" << last_search_results.size() << " registro(s)) ---" << std::endl;
                    std::string confirm_remove = OpsMenuHelpers::get_line_input_ops("Digite 's' para confirmar a remocao de todos estes registros ou qualquer outra tecla para cancelar: ");
                    if (confirm_remove == "s" || confirm_remove == "S") {
                        int removed_count_ds = 0;
                        int removed_count_raw = 0;
                        for (const auto& rec_to_remove : last_search_results) {
                            bool ds_removed_current = false;
                            if (std::is_same<DataStructureType, HashTable>::value) {
                                auto& ht_ref = reinterpret_cast<HashTable&>(data_structure);
                                ds_removed_current = ht_ref.remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city);
                            } else if (std::is_same<DataStructureType, DoublyLinkedList>::value) {
                                ds_removed_current = reinterpret_cast<DoublyLinkedList&>(data_structure).remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city, rec_to_remove.country);
                            } else if (std::is_same<DataStructureType, AVLTree>::value) {
                                ds_removed_current = reinterpret_cast<AVLTree&>(data_structure).remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city, rec_to_remove.country);
                            } else if (std::is_same<DataStructureType, KDTree>::value) {
                                ds_removed_current = reinterpret_cast<KDTree&>(data_structure).remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city, rec_to_remove.country);
                            } else if (std::is_same<DataStructureType, SkipList>::value) {
                                ds_removed_current = reinterpret_cast<SkipList&>(data_structure).remove_record(rec_to_remove.date, rec_to_remove.time, rec_to_remove.city, rec_to_remove.country);
                            }
                            if(ds_removed_current) removed_count_ds++;

                            for (auto it_raw = all_raw_records_ref.begin(); it_raw != all_raw_records_ref.end(); ) {
                                if (it_raw->date == rec_to_remove.date && it_raw->time == rec_to_remove.time &&
                                    it_raw->city == rec_to_remove.city && it_raw->country == rec_to_remove.country) { // Comparação completa para a lista raw
                                    it_raw = all_raw_records_ref.erase(it_raw);
                                    removed_count_raw++;
                                    break;
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
                    last_search_results.clear(); // Limpa após a tentativa de remoção
                    can_remove_from_search = false; // Reseta o flag
                } else { // A opção 5 foi escolhida (diretamente ou por erro), mas não está disponível
                    std::cout << "Opcao 5 (Remover da Ultima Busca) nao esta disponivel. "
                              << "Realize uma busca com resultados primeiro." << std::endl;
                }
                break;
            }
            case 6: {
                if (std::is_same<DataStructureType, KDTree>::value) {
                    // (Código original da KD-Tree para case 6 mantido)
                    auto& kd_ref = reinterpret_cast<KDTree&>(data_structure);
                    std::cout << "\n--- Buscar por Range Geografico (KD-Tree) ---" << std::endl;
                    double min_lat_val = 0.0, max_lat_val = 0.0, min_lon_val = 0.0, max_lon_val = 0.0;
                    std::string temp_in;

                    std::cout << "Digite a Latitude Minima: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if (!KDTreeHelpers::try_string_to_double(temp_in, min_lat_val, true, "Latitude Minima")) { break; }

                    std::cout << "Digite a Latitude Maxima: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if (!KDTreeHelpers::try_string_to_double(temp_in, max_lat_val, true, "Latitude Maxima")) { break; }

                    std::cout << "Digite a Longitude Minima: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if (!KDTreeHelpers::try_string_to_double(temp_in, min_lon_val, true, "Longitude Minima")) { break; }

                    std::cout << "Digite a Longitude Maxima: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if (!KDTreeHelpers::try_string_to_double(temp_in, max_lon_val, true, "Longitude Maxima")) { break; }

                    last_search_results = kd_ref.search_by_range(min_lat_val, max_lat_val, min_lon_val, max_lon_val);
                    OpsMenuHelpers::display_op_search_results(last_search_results,
                        "Range: Lat[" + std::to_string(min_lat_val) + "," + std::to_string(max_lat_val) +
                        "] Lon[" + std::to_string(min_lon_val) + "," + std::to_string(max_lon_val) + "]");
                    if (!last_search_results.empty()) can_remove_from_search = true; else can_remove_from_search = false;

                } else if (std::is_same<DataStructureType, HashTable>::value) {
                    // (Código original da HashTable para case 6 mantido)
                    auto& ht_ref = reinterpret_cast<HashTable&>(data_structure);
                    ht_ref.display_table_distribution();
                } else { // Opção 6 escolhida, mas não é KDTree nem HashTable
                    std::cout << "Opcao 6 nao esta disponivel para " << structure_name << "." << std::endl;
                }
                break;
            }
            case 7: {
                if (std::is_same<DataStructureType, KDTree>::value) {
                    // (Código original da KD-Tree para case 7 mantido)
                    auto& kd_ref = reinterpret_cast<KDTree&>(data_structure);
                    std::cout << "\n--- Buscar Vizinho Mais Proximo (KD-Tree) ---" << std::endl;
                    double q_lat_val = 0.0, q_lon_val = 0.0;
                    std::string temp_in;

                    std::cout << "Digite a Latitude do Ponto de Consulta: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if(!KDTreeHelpers::try_string_to_double(temp_in, q_lat_val, true, "Latitude Consulta")) { break;}

                    std::cout << "Digite a Longitude do Ponto de Consulta: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if(!KDTreeHelpers::try_string_to_double(temp_in, q_lon_val, true, "Longitude Consulta")) { break;}

                    EarthquakeRecord* nearest = const_cast<EarthquakeRecord*>(kd_ref.search_nearest_neighbor(q_lat_val, q_lon_val));
                    last_search_results.clear();
                    if (nearest) {
                        last_search_results.push_back(*nearest);
                        OpsMenuHelpers::display_op_search_results(last_search_results,
                            "Vizinho Mais Proximo de (" + std::to_string(q_lat_val) + "," + std::to_string(q_lon_val) + ")");
                        can_remove_from_search = true;
                    } else {
                        std::cout << "Nenhum vizinho encontrado (KD-Tree vazia ou ponto nao encontrado)." << std::endl;
                        can_remove_from_search = false;
                    }
                } else { // Opção 7 escolhida, mas não é KDTree
                    std::cout << "Opcao 7 so esta disponivel para a KD-Tree." << std::endl;
                }
                break;
            }
            case 8: {
                if (std::is_same<DataStructureType, KDTree>::value) {
                    // (Código original da KD-Tree para case 8 mantido)
                    auto& kd_ref = reinterpret_cast<KDTree&>(data_structure);
                    std::cout << "\n--- Buscar K Vizinhos Mais Proximos (KD-Tree) ---" << std::endl;
                    double q_lat_k_val = 0.0, q_lon_k_val = 0.0;
                    unsigned k_val_val = 0;
                    std::string temp_in;

                    std::cout << "Digite a Latitude do Ponto de Consulta: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if(!KDTreeHelpers::try_string_to_double(temp_in, q_lat_k_val, true, "Latitude Consulta K")) { break;}

                    std::cout << "Digite a Longitude do Ponto de Consulta: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    if(!KDTreeHelpers::try_string_to_double(temp_in, q_lon_k_val, true, "Longitude Consulta K")) { break;}

                    std::cout << "Digite o valor de K: "; temp_in = OpsMenuHelpers::get_line_input_ops("");
                    try {
                        k_val_val = std::stoul(temp_in);
                        if (k_val_val == 0) throw std::invalid_argument("K must be > 0");
                    } catch (const std::exception& e) {
                         std::cout << "Entrada invalida para K (deve ser um numero inteiro > 0): " << e.what() << std::endl; break;
                    }

                    last_search_results = kd_ref.search_k_nearest_neighbors(q_lat_k_val, q_lon_k_val, k_val_val);
                    OpsMenuHelpers::display_op_search_results(last_search_results,
                        std::to_string(k_val_val) + " Vizinhos Mais Proximos de (" + std::to_string(q_lat_k_val) + "," + std::to_string(q_lon_k_val) + ")");
                    if (!last_search_results.empty()) can_remove_from_search = true; else can_remove_from_search = false;
                } else { // Opção 8 escolhida, mas não é KDTree
                    std::cout << "Opcao 8 so esta disponivel para a KD-Tree." << std::endl;
                }
                break;
            }
            case 0:
                std::cout << "Voltando ao menu anterior..." << std::endl;
                break;
            default: // Para qualquer outra entrada numérica que não seja 0-8
                std::cout << "Opcao invalida. Tente novamente." << std::endl;
                break;
        }
    } while (choice != 0);
}

#endif // OPERATIONS_MENU_H