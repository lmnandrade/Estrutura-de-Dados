#include <iostream>
#include <string>
#include <vector>
#include <limits>    // Para std::numeric_limits
#include <map>       // Para std::map
#include <algorithm> // Para std::replace_if (no nome do arquivo de benchmark)

// Seus includes de projeto
#include "csv_reader.h"         // Para EarthquakeRecord e read_earthquake_csv
#include "lista_dupla_ligada.h" // Para DoublyLinkedList
// #include "avl_tree.h"        // Descomente quando tiver
// #include "hash_table.h"      // Descomente quando tiver
// #include "skip_list.h"       // Descomente quando tiver
// #include "kd_tree.h"         // Descomente quando tiver
#include "classification.h"     // Para CountryRiskProfile e classify_countries_by_risk
#include "operations_menu.h"    // Para menu_monitor_terremotos e menu_alerta_terremoto
#include "benchmark.h"          // Para BenchmarkRunner

// Função auxiliar para limpar o buffer de entrada (pode ser movida para um utils.h)
void clear_main_input_buffer() {
    std::cin.clear(); // Limpa flags de erro do cin
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Função auxiliar para cadastro de emails (simples)
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
            bool exists = false;
            for(const auto& existing_email : emails) {
                if (existing_email == email_input) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                emails.push_back(email_input);
                std::cout << "Email '" << email_input << "' adicionado." << std::endl;
            } else {
                std::cout << "Email '" << email_input << "' ja cadastrado." << std::endl;
            }
        } else {
            std::cout << "Formato de email invalido. Tente novamente." << std::endl;
        }
    }
    std::cout << "Cadastro de emails concluido." << std::endl;
}


int main(int argc, char *argv[]) {
    int main_choice;
    std::string csv_filepath = "C:/Users/letic/OneDrive/Documentos/Repositorio-github/Estrutura-de-Dados/earthquake_dataset.csv"; // CONFIRME O NOME DO SEU ARQUIVO

    std::vector<EarthquakeRecord> all_earthquake_data; // Usa EarthquakeRecord do seu csv_reader.h
    std::map<std::string, CountryRiskProfile> country_risk_profiles;
    std::vector<std::string> registered_emails;

    std::string benchmark_scenario_name = "Baseline_NoRestriction";
    if (argc > 1) {
        benchmark_scenario_name = argv[1];
        std::cout << "INFO: Executando sob o cenario de restricao de benchmark: " << benchmark_scenario_name << std::endl;
    }

    std::cout << "INFO: Tentando carregar dados do arquivo: " << csv_filepath << std::endl;
    // CORREÇÃO: Usando a função read_earthquake_csv como definida em seu csv_reader.h
    all_earthquake_data = read_earthquake_csv(csv_filepath); 

    if (all_earthquake_data.empty()) {
        std::cerr << "ERRO: Nao foi possivel carregar os dados do CSV ou o arquivo esta vazio/com erro." << std::endl;
        std::cerr << "Verifique o caminho do arquivo ('" << csv_filepath << "') e seu conteudo. O programa sera encerrado." << std::endl;
        return 1;
    }
    std::cout << "INFO: " << all_earthquake_data.size() << " registros carregados com sucesso do arquivo CSV." << std::endl;

    std::cout << "\nINFO: Classificando paises por risco..." << std::endl;
    country_risk_profiles = classify_countries_by_risk(all_earthquake_data);

    // Usa DoublyLinkedList como definido em seu lista_dupla_ligada.h
    DoublyLinkedList earthquake_dll; 
    // AVLTree earthquake_avl; 
    // HashTable earthquake_ht; 
    // SkipList earthquake_sl; 
    // KDTree earthquake_kdt; 

    std::cout << "\nINFO: Populando estruturas de dados com os registros carregados..." << std::endl;
    for (const auto& record : all_earthquake_data) {
        earthquake_dll.insert_record(record); // insert_record é um método de DoublyLinkedList
        // earthquake_avl.insert_record(record);
        // ... etc ...
    }
    std::cout << "INFO: Estruturas de dados populadas." << std::endl;

    do {
        std::cout << "\n========= MENU PRINCIPAL - ANALISADOR DE TERREMOTOS =========" << std::endl;
        std::cout << "1. Monitorar Terremotos (Usar Estruturas de Dados)" << std::endl;
        std::cout << "2. Alerta de Terremoto" << std::endl;
        std::cout << "3. Ver Classificacao de Risco dos Paises" << std::endl;
        std::cout << "4. Cadastrar Email para Alertas" << std::endl;
        std::cout << "5. Calculos Estatisticos (Nao implementado)" << std::endl;
        std::cout << "8. EXECUTAR BENCHMARKS" << std::endl;
        std::cout << "0. Sair do Programa" << std::endl;
        std::cout << "==============================================================" << std::endl;
        std::cout << "Sua escolha: ";
        
        if (!(std::cin >> main_choice)) {
            std::cout << "ERRO: Entrada invalida. Por favor, insira um numero." << std::endl;
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
                std::cout << "2. Arvore AVL (Nao implementado)" << std::endl;
                // ... outras opções ...
                std::cout << "0. Voltar ao Menu Principal" << std::endl;
                std::cout << "Sua escolha: ";
                
                if (!(std::cin >> structure_choice_monitor)) {
                     std::cout << "ERRO: Entrada invalida." << std::endl;
                     clear_main_input_buffer();
                     break;
                }
                clear_main_input_buffer();

                std::vector<EarthquakeRecord> current_data_for_classification;

                switch (structure_choice_monitor) {
                    case 1:
                        std::cout << "\n--- Usando Lista Duplamente Ligada para Monitoramento ---" << std::endl;
                        menu_monitor_terremotos(earthquake_dll, country_risk_profiles, all_earthquake_data);
                        // CORREÇÃO: Usando get_all_records_vector como definido em sua lista_dupla_ligada.h
                        current_data_for_classification = earthquake_dll.get_all_records_vector(); 
                        country_risk_profiles = classify_countries_by_risk(current_data_for_classification);
                        std::cout << "INFO: Perfis de risco dos paises atualizados apos modificacoes na lista." << std::endl;
                        break;
                    // ... outros cases ...
                    case 0:
                        break;
                    default:
                        std::cout << "Opcao de estrutura invalida." << std::endl;
                }
                break;
            }
            case 2:
                menu_alerta_terremoto(country_risk_profiles, registered_emails, all_earthquake_data);
                break;
            case 3:
                print_country_risk_profiles(country_risk_profiles);
                break;
            case 4:
                manage_email_cadastro(registered_emails);
                break;
            case 5:
                 std::cout << "Calculos estatisticos (estatistica.cpp) ainda nao implementados no menu." << std::endl;
                break;
            case 8: {
                if (all_earthquake_data.empty()) {
                    std::cout << "ERRO: Dataset vazio. Nao e possivel executar benchmarks." << std::endl;
                    break;
                }
                BenchmarkRunner runner(all_earthquake_data, benchmark_scenario_name);
                runner.run_all_benchmarks();
                
                std::string results_filename = "benchmark_results_" + benchmark_scenario_name + ".csv";
                std::replace_if(results_filename.begin(), results_filename.end(), 
                                [](char c){ return !std::isalnum(c) && c != '_' && c != '.'; }, 
                                '_');
                runner.save_results_to_csv(results_filename);
                break;
            }
            case 0:
                std::cout << "Encerrando o programa..." << std::endl;
                break;
            default:
                std::cout << "ERRO: Opcao principal invalida. Tente novamente." << std::endl;
        }
    } while (main_choice != 0);

   // ...existing code...
// Salva os registros atualizados no CSV ao sair
save_earthquake_csv(csv_filepath, earthquake_dll.get_all_records_vector());
return 0;
}