#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>   // Para std::numeric_limits
#include <iomanip>  // Para std::setw, std::fixed, std::setprecision
#include <algorithm>

#include "csv_reader.h"         // Para EarthquakeRecord, read_earthquake_csv, displayRecord
#include "classification.h"     // Para CountryRiskProfile, classify_countries_by_risk
#include "operations_menu.h"    // Para menu_monitor_terremotos
#include "lista_dupla_ligada.h"
#include "avl_tree.h"
#include "hash_table.h"
#include "kd_tree.h"
#include "skip_list.h"
#include "perfect_hash.h"       
#include "alerta.h"
#include "cadastro.h"
#include "benchmark.h"
#include "statistic.h"          
#include "filtragem.h"
#include "tendencias.h"
#include "benchmark_menu.h"

int main() {
    const std::string csv_filepath = "C:/Users/letic/OneDrive/Documentos/Repositorio-github/Estrutura-de-Dados/earthquake_dataset.csv";
    std::vector<std::string> registered_emails;

    std::cout << "Bem-vindo ao Sistema de Monitoramento de Terremotos Globais!" << std::endl;
    std::cout << "Lendo o dataset de: " << csv_filepath << std::endl;
std::vector<EarthquakeRecord> raw_records = read_earthquake_csv(csv_filepath);

if (raw_records.empty()) {
    std::cerr << "ERRO: Nenhum registro foi lido do arquivo CSV ou o arquivo nao foi encontrado." << std::endl;
    std::cerr << "Verifique o caminho: " << csv_filepath << std::endl;
    std::cout << "Pressione Enter para sair...";
    std::cin.get();
    return 1;
}
std::cout << "\nIniciando filtragem dos dados..." << std::endl;
int removed_during_sanitization = 0;
// A variável 'all_records' agora conterá os dados limpos.
std::vector<EarthquakeRecord> all_records = sanitize_earthquake_records(raw_records, removed_during_sanitization, true);

if (removed_during_sanitization > 0) {
    std::cout << "Filtragem concluida. " << removed_during_sanitization << " registros foram removidos." << std::endl;
}
std::cout << all_records.size() << " registros validos permanecem apos a filtragem." << std::endl;

if (all_records.empty() && !raw_records.empty()) {
    std::cerr << "ALERTA: Todos os registros foram removidos durante a filtragem." << std::endl;
    std::cout << "Pressione Enter para continuar (o programa pode nao funcionar como esperado)...";
    std::cin.get();
    // Você pode decidir sair aqui se for crítico não ter dados.
} else if (all_records.empty() && raw_records.empty()){
     // Já tratado pelo if anterior de raw_records.empty()
}


    std::cout << "\nCalculando perfis de risco dos paises com base nos dados sanitizados..." << std::endl;
    // 'country_risks' e todas as estruturas subsequentes usarão 'all_records' (os dados limpos)
    std::map<std::string, CountryRiskProfile> country_risks = classify_countries_by_risk(all_records);
    if (country_risks.empty() && !all_records.empty()){
         std::cout << "AVISO: Nenhum perfil de risco de pais foi calculado. Verifique a implementacao de 'classify_countries_by_risk'." << std::endl;
    } else if (!country_risks.empty()){
        std::cout << country_risks.size() << " paises tiveram seus perfis de risco calculados/identificados." << std::endl;
    }
const double python_b1 = 243.7911;       // Coeficiente da Magnitude
const double python_b0 = -845.0397;      // Intercepto
const double python_r_squared = 0.5157;
const double python_mse = 9120.2211;
const std::string python_formula = "Impact Score = 243.7911 * Magnitude + -845.0397";
PredefinedLinearModel python_model(python_b0, python_b1, python_r_squared, python_mse, python_formula);

    int main_choice;
    do {
        std::cout << "\n===== MENU PRINCIPAL =====" << std::endl;
        std::cout << "1. Selecionar Estrutura de Dados para Monitoramento" << std::endl;
        std::cout << "2. Alerta de Terremoto" << std::endl;
        std::cout << "3. Calculos Estatisticos sobre os Dados" << std::endl;
        std::cout << "4. Previsao de Impact Score com base na magnitude" << std::endl;
        std::cout << "5. Cadastro para Alertas de Terremoto" << std::endl;
        std::cout << "6. Executar Benchmarks" << std::endl;
        std::cout << "0. Sair do Programa" << std::endl;
        std::cout << "Sua escolha: ";

        if (!(std::cin >> main_choice)) {
            std::cout << "ERRO: Entrada invalida. Por favor, insira um numero." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            main_choice = -1;
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (main_choice) {
            case 1: { // Selecionar Estrutura de Dados
                // ... (código do case 1 permanece o mesmo)
                int ds_choice;
                std::cout << "\n--- Selecionar Estrutura de Dados ---" << std::endl;
                std::cout << "1. Lista Duplamente Ligada" << std::endl;
                std::cout << "2. Arvore AVL" << std::endl;
                std::cout << "3. Tabela Hash" << std::endl;
                std::cout << "4. KD-Tree" << std::endl;
                std::cout << "5. Skip-List" << std::endl;
                std::cout << "6. Tabela Hash Perfeita" << std::endl;
                std::cout << "0. Voltar ao Menu Principal" << std::endl;
                std::cout << "Escolha a estrutura: ";

                if (!(std::cin >> ds_choice)) {
                    std::cout << "ERRO: Entrada invalida." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;
                }
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (ds_choice == 1) {
                    DoublyLinkedList dll;
                    std::cout << "Populando Lista Duplamente Ligada..." << std::endl;
                    for (const auto& rec : all_records) { dll.insert_record(rec); }
                    std::cout << "Lista Duplamente Ligada populada com " << dll.get_count() << " registros." << std::endl;
                    menu_monitor_terremotos(dll, "Lista Duplamente Ligada", country_risks, all_records);
                } else if (ds_choice == 2) {
                    AVLTree avl;
                    std::cout << "Populando Arvore AVL..." << std::endl;
                    for (const auto& rec : all_records) { avl.insert_record(rec); }
                    std::cout << "Arvore AVL populada com " << avl.get_count() << " registros." << std::endl;
                    menu_monitor_terremotos(avl, "Arvore AVL", country_risks, all_records);
                } else if (ds_choice == 3) {
                    HashTable ht(all_records.size());
                    std::cout << "Populando Tabela Hash..." << std::endl;
                    for (const auto& rec : all_records) { ht.insert_record(rec); }
                    std::cout << "Tabela Hash populada com " << ht.get_count() << " registros." << std::endl;
                    menu_monitor_terremotos(ht, "Tabela Hash", country_risks, all_records);
                } else if (ds_choice == 4) {
                    KDTree kdt;
                    std::cout << "Populando KD-Tree..." << std::endl;
                    int populated_kdt_count = 0;
                    for (const auto& rec : all_records) {
                        double lat_val, lon_val;
                        if (KDTreeHelpers::try_string_to_double(rec.latitude, lat_val, false) &&
                            KDTreeHelpers::try_string_to_double(rec.longitude, lon_val, false)) {
                            kdt.insert_record(rec);
                            populated_kdt_count++;
                        }
                    }
                    std::cout << "KD-Tree populada com " << populated_kdt_count << " registros validos." << std::endl;
                    if (static_cast<size_t>(populated_kdt_count) < all_records.size()) {
                         std::cout << (all_records.size() - populated_kdt_count) << " registros foram ignorados na KD-Tree devido a coordenadas invalidas." << std::endl;
                    }
                    menu_monitor_terremotos(kdt, "KD-Tree", country_risks, all_records);
                }
                else if (ds_choice == 5) {
                    SkipList sl;
                    std::cout << "Populando Skip-List..." << std::endl;
                    for (const auto& rec : all_records) { sl.insert_record(rec); }
                    std::cout << "Skip-List populada com " << sl.get_count() << " registros." << std::endl;
                    menu_monitor_terremotos(sl, "Skip-List", country_risks, all_records);
                }
                else if (ds_choice == 6) {
                    std::cout << "Construindo Tabela Hash Perfeita..." << std::endl;
                    PerfectHashTable pht(all_records.size());
                    pht.build_table(all_records);
                    std::cout << "Tabela Hash Perfeita construida com " << pht.get_count() << " registros." << std::endl;
                    menu_monitor_terremotos(pht, "Tabela Hash Perfeita", country_risks, all_records);
                }
                else if (ds_choice == 0) {
                    // Voltar
                } else {
                    std::cout << "Opcao de estrutura invalida." << std::endl;
                }
                break;
            }
            case 2:
                alerta_terremoto(country_risks, registered_emails, all_records);
                break;
            case 3:
                menu_estatisticas(all_records);
                break;
            case 4:
                  show_trends_menu_with_predefined_model(all_records, python_model);
                break;
            case 5:
                manage_email_cadastro(registered_emails);
                break;
            case 6: 
                if (all_records.empty()) {
                    std::cout << "Nao e possivel executar benchmarks: o dataset principal esta vazio." << std::endl;
                } else {
                    benchmark_menu(all_records);
                }
                break;
            case 0:
                std::cout << "Encerrando o programa..." << std::endl;
                break;
            default:
                std::cout << "Opcao invalida. Tente novamente." << std::endl;
        }
        
        // Adicionado "Pressione Enter para continuar..." para os casos que não saem ou não têm submenu
        if (main_choice != 0 && main_choice != 1 && main_choice != 2 && main_choice != 3 && main_choice != 4 && main_choice != 7) {
             if (main_choice == 8 && all_records.empty()) {
                // Caso especial para benchmark com dataset vazio, já tem sua mensagem
             } else if (main_choice == 8 && !all_records.empty()){
                // Caso especial para benchmark concluído
             }
             else { // Para opções 5, 6 e default
                std::cout << "Pressione Enter para continuar...";
             }
             // std::cin.get(); // O ignore no início do loop do/while já está tratando o newline.
                           // Se você ainda precisar de um "pause", pode descomentar, mas teste primeiro.
                           // É provável que o std::cin.ignore no loop principal seja suficiente.
        }

    } while (main_choice != 0);

    std::cout << "Obrigado por usar o sistema!" << std::endl;
    return 0;
}