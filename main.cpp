#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <map>
#include <algorithm>

#include "csv_reader.h"
#include "lista_dupla_ligada.h"
#include "avl_tree.h"
#include "kd_tree.h"
#include "classification.h"
#include "operations_menu.h"
#include "alerta.h"  // Incluindo o header para alerta_terremoto
#include "benchmark.h"

void clear_main_input_buffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void manage_email_cadastro(std::vector<std::string>& emails) {
    std::string email_input;
    std::cout << "\n--- Cadastro de Email para Alertas ---" << std::endl;
    std::cout << "Emails atualmente cadastrados: ";
    if (emails.empty()) {
        std::cout << "Nenhum." << std::endl;
    } else {
        for (size_t i = 0; i < emails.size(); ++i) {
            std::cout << emails[i] << (i == emails.size() - 1 ? "" : ", ");
        }
        std::cout << std::endl;
    }
    while (true) {
        std::cout << "Digite um email para adicionar (ou 'fim' para concluir): ";
        std::getline(std::cin, email_input);

        if (email_input.empty() || email_input == "fim") {
            break;
        }

        if (email_input.find('@') != std::string::npos && email_input.find('.') != std::string::npos) {
            if (std::find(emails.begin(), emails.end(), email_input) == emails.end()) {
                emails.push_back(email_input);
                std::cout << "Email '" << email_input << "' adicionado." << std::endl;
            } else {
                std::cout << "Email '" << email_input << "' já cadastrado." << std::endl;
            }
        } else {
            std::cout << "Formato de email inválido. Tente novamente." << std::endl;
        }
    }
    std::cout << "Cadastro de emails concluído." << std::endl;
}

int main(int argc, char *argv[]) {
    int main_choice;
    std::string csv_filepath = "C:/Users/letic/OneDrive/Documentos/Repositorio-github/Estrutura-de-Dados/earthquake_dataset.csv";

    std::vector<EarthquakeRecord> all_earthquake_data;
    std::map<std::string, CountryRiskProfile> country_risk_profiles;
    std::vector<std::string> registered_emails;

    std::string benchmark_scenario_name = "Baseline_NoRestriction";
    if (argc > 1) {
        benchmark_scenario_name = argv[1];
        std::cout << "INFO: Executando sob o cenario de restrição de benchmark: " << benchmark_scenario_name << std::endl;
    }

    std::cout << "INFO: Tentando carregar dados do arquivo: " << csv_filepath << std::endl;
    all_earthquake_data = read_earthquake_csv(csv_filepath);

    if (all_earthquake_data.empty()) {
        std::cerr << "ERRO: Não foi possível carregar os dados do CSV ou o arquivo está vazio/com erro. Encerrando." << std::endl;
        return 1;
    }
    std::cout << "INFO: " << all_earthquake_data.size() << " registros carregados com sucesso do arquivo CSV." << std::endl;

    std::cout << "\nINFO: Classificando países por risco..." << std::endl;
    country_risk_profiles = classify_countries_by_risk(all_earthquake_data);

    DoublyLinkedList earthquake_dll;
    AVLTree earthquake_avl;
    KDTree earthquake_kdt;

    std::cout << "\nINFO: Populando estruturas de dados com os registros carregados..." << std::endl;
    for (const auto& record : all_earthquake_data) {
        earthquake_dll.insert_record(record);
        earthquake_avl.insert_record(record);
        earthquake_kdt.insert_record(record);
    }
    std::cout << "INFO: Estruturas de dados populadas." << std::endl;

    do {
        std::cout << "\n========= MENU PRINCIPAL - ANALISADOR DE TERREMOTOS =========" << std::endl;
        std::cout << "1. Monitorar Terremotos (Usar Estruturas de Dados)" << std::endl;
        std::cout << "2. Alerta de Terremoto" << std::endl;
        std::cout << "3. Ver Classificação de Risco dos Países" << std::endl;
        std::cout << "4. Cadastrar Email para Alertas" << std::endl;
        std::cout << "5. Cálculos Estatísticos (Não implementado)" << std::endl;
        std::cout << "8. EXECUTAR BENCHMARKS" << std::endl;
        std::cout << "0. Sair do Programa" << std::endl;
        std::cout << "==============================================================" << std::endl;
        std::cout << "Sua escolha: ";

        if (!(std::cin >> main_choice)) {
            std::cout << "ERRO: Entrada inválida. Por favor, insira um número." << std::endl;
            clear_main_input_buffer();
            main_choice = -1;
            continue;
        }
        clear_main_input_buffer();

        switch (main_choice) {
            case 1: {
                int structure_choice_monitor;
                std::cout << "\n--- Monitorar Terremotos ---" << std::endl;
                std::cout << "Qual estrutura de dados deseja usar?" << std::endl;
                std::cout << "1. Lista Duplamente Ligada" << std::endl;
                std::cout << "2. Árvore AVL" << std::endl;
                std::cout << "3. KD-Tree" << std::endl;
                std::cout << "0. Voltar ao Menu Principal" << std::endl;
                std::cout << "Sua escolha: ";

                if (!(std::cin >> structure_choice_monitor)) {
                    std::cout << "ERRO: Entrada inválida." << std::endl;
                    clear_main_input_buffer();
                    break;
                }
                clear_main_input_buffer();

                bool profiles_updated = false;
                switch (structure_choice_monitor) {
                    case 1:
                        menu_monitor_terremotos(earthquake_dll, "Lista Duplamente Ligada", country_risk_profiles, all_earthquake_data);
                        profiles_updated = true;
                        break;
                    case 2:
                        menu_monitor_terremotos(earthquake_avl, "Árvore AVL", country_risk_profiles, all_earthquake_data);
                        profiles_updated = true;
                        break;
                    case 3:
                        menu_monitor_terremotos(earthquake_kdt, "KD-Tree", country_risk_profiles, all_earthquake_data);
                        profiles_updated = true;
                        break;
                    case 0:
                        break;
                    default:
                        std::cout << "Opção de estrutura inválida." << std::endl;
                }

                if (profiles_updated) {
                    country_risk_profiles = classify_countries_by_risk(all_earthquake_data);
                    std::cout << "INFO: Perfis de risco dos países atualizados." << std::endl;
                }
                break;
            }
            case 2:
                alerta_terremoto(country_risk_profiles, registered_emails, all_earthquake_data);
                break;
            case 3:
                print_country_risk_profiles(country_risk_profiles);
                break;
            case 4:
                manage_email_cadastro(registered_emails);
                break;
            case 5:
                std::cout << "Cálculos estatísticos (estatistica.cpp) ainda não implementados no menu." << std::endl;
                break;
            case 8: {
                if (all_earthquake_data.empty()) {
                    std::cout << "ERRO: Dataset vazio. Não é possível executar benchmarks." << std::endl;
                    break;
                }
                BenchmarkRunner runner(all_earthquake_data, benchmark_scenario_name);
                runner.run_all_benchmarks();

                std::string results_filename = "benchmark_results_" + benchmark_scenario_name + ".csv";
                std::replace_if(results_filename.begin(), results_filename.end(),
                                [](char c) { return !std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '.'; },
                                '_');
                runner.save_results_to_csv(results_filename);
                break;
            }
            case 0:
                std::cout << "Encerrando o programa..." << std::endl;
                break;
            default:
                std::cout << "ERRO: Opção principal inválida. Tente novamente." << std::endl;
        }
    } while (main_choice != 0);

    std::cout << "Salvando " << all_earthquake_data.size() << " registros no arquivo CSV antes de sair..." << std::endl;
    save_earthquake_csv(csv_filepath, all_earthquake_data);
    std::cout << "Dados salvos. Programa encerrado." << std::endl;

    return 0;
}