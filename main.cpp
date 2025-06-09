#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <limits>   // Para std::numeric_limits
#include <iomanip>  // Para std::setw, std::fixed, std::setprecision
#include <algorithm>
#include <fstream> // Para std::ifstream

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
#include "benchmark.h"
#include "statistic.h"
#include "filtragem.h" // Certifique-se que sanitize_earthquake_records está aqui ou em um utils
#include "tendencias.h"
#include "benchmark_menu.h"

// Função auxiliar para verificar se um arquivo existe
bool file_exists(const std::string& filepath) {
    std::ifstream f(filepath.c_str());
    return f.good();
}

int main() {
std::string csv_filepath;
std::cout << "Digite o caminho do arquivo CSV (ou pressione Enter para usar 'earthquake_dataset.csv'): ";
std::getline(std::cin, csv_filepath);
if (csv_filepath.empty()) {
    csv_filepath = "earthquake_dataset.csv";
}

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

    // --- Classificação de Risco e Modelo de Tendência ---
    std::cout << "\nCalculando perfis de risco dos paises com base nos dados sanitizados..." << std::endl;
    std::map<std::string, CountryRiskProfile> country_risks = classify_countries_by_risk(all_records);
    if (country_risks.empty() && !all_records.empty()){
         std::cout << "AVISO: Nenhum perfil de risco de pais foi calculado. Verifique a implementacao de 'classify_countries_by_risk' ou se os dados de paises estao presentes e validos." << std::endl;
    } else if (!country_risks.empty()){
        std::cout << country_risks.size() << " paises tiveram seus perfis de risco calculados/identificados." << std::endl;
    }

    const double python_b1 = 243.7911;
    const double python_b0 = -845.0397;
    const double python_r_squared = 0.5157;
    const double python_mse = 9120.2211;
    const std::string python_formula = "Impact Score = 243.7911 * Magnitude + -845.0397";
    PredefinedLinearModel python_model(python_b0, python_b1, python_r_squared, python_mse, python_formula);

    // --- Menu Principal ---
    int main_choice;
    do {
        std::cout << "\n===== MENU PRINCIPAL =====" << std::endl;
        std::cout << "1. Selecionar Estrutura de Dados para Monitoramento" << std::endl;
        std::cout << "2. Alerta de Terremoto" << std::endl;
        std::cout << "3. Calculos Estatisticos sobre os Dados" << std::endl;
        std::cout << "4. Previsao de Impact Score com base na magnitude" << std::endl;
        std::cout << "5. Ver classificacao de risco dos Paises" << std::endl;
        std::cout << "6. Executar Benchmarks das Estruturas" << std::endl;
        std::cout << "0. Sair do Programa" << std::endl;
        std::cout << "Sua escolha: ";

        if (!(std::cin >> main_choice)) {
            std::cout << "ERRO: Entrada invalida. Por favor, insira um numero." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            main_choice = -1; // Para continuar o loop
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Limpa o buffer do Enter após o número

        switch (main_choice) {
            case 1: {
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

                // Verifica se all_records está vazio ANTES de tentar popular as estruturas
                if (all_records.empty() && ds_choice != 0) {
                    std::cout << "AVISO: O conjunto de dados validos esta vazio. Nao e possivel popular a estrutura." << std::endl;
                    std::cout << "Verifique o arquivo CSV ou os criterios de sanitizacao." << std::endl;
                    std::cout << "Pressione Enter para voltar ao menu...";
                    std::cin.get(); // Pausa
                    break; // Sai do case 1, volta ao menu principal
                }


                switch (ds_choice) {
                    case 1: {
                        DoublyLinkedList dll;
                        std::cout << "Populando Lista Duplamente Ligada..." << std::endl;
                        for (const auto& rec : all_records) { dll.insert_record(rec); }
                        std::cout << "Lista Duplamente Ligada populada com " << dll.get_count() << " registros." << std::endl;
                        menu_monitor_terremotos(dll, "Lista Duplamente Ligada", country_risks, all_records);
                        break;
                    }
                    case 2: {
                        AVLTree avl;
                        std::cout << "Populando Arvore AVL..." << std::endl;
                        for (const auto& rec : all_records) { avl.insert_record(rec); }
                        std::cout << "Arvore AVL populada com " << avl.get_count() << " registros." << std::endl;
                        menu_monitor_terremotos(avl, "Arvore AVL", country_risks, all_records);
                        break;
                    }
                    case 3: {
                        HashTable ht(all_records.size() > 0 ? all_records.size() : 100); // Evita tamanho 0
                        std::cout << "Populando Tabela Hash..." << std::endl;
                        for (const auto& rec : all_records) { ht.insert_record(rec); }
                        std::cout << "Tabela Hash populada com " << ht.get_count() << " registros." << std::endl;
                        menu_monitor_terremotos(ht, "Tabela Hash", country_risks, all_records);
                        break;
                    }
                    case 4: {
                        KDTree kdt;
                        std::cout << "Populando KD-Tree..." << std::endl;
                        int populated_kdt_count = 0;
                        for (const auto& rec : all_records) {
                            double lat_val, lon_val;
                            // Usando try_string_to_double de KDTreeHelpers, assumindo que está disponível
                            // Se não, você precisaria de uma função similar aqui ou em utils.h
                            if (KDTreeHelpers::try_string_to_double(rec.latitude, lat_val, "latitude", true) &&
                                KDTreeHelpers::try_string_to_double(rec.longitude, lon_val, "longitude", true)) {
                                kdt.insert_record(rec);
                                populated_kdt_count++;
                            }
                        }
                        std::cout << "KD-Tree populada com " << populated_kdt_count << " registros com coordenadas validas." << std::endl;
                        if (static_cast<size_t>(populated_kdt_count) < all_records.size()) {
                             std::cout << (all_records.size() - populated_kdt_count) << " registros foram ignorados na KD-Tree devido a coordenadas invalidas ou ausentes." << std::endl;
                        }
                        menu_monitor_terremotos(kdt, "KD-Tree", country_risks, all_records);
                        break;
                    }
                    case 5: {
                        SkipList sl;
                        std::cout << "Populando Skip-List..." << std::endl;
                        for (const auto& rec : all_records) { sl.insert_record(rec); }
                        std::cout << "Skip-List populada com " << sl.get_count() << " registros." << std::endl;
                        menu_monitor_terremotos(sl, "Skip-List", country_risks, all_records);
                        break;
                    }
                    case 6: {
                        std::cout << "Construindo Tabela Hash Perfeita..." << std::endl;
                        PerfectHashTable pht(all_records.size() > 0 ? all_records.size() : 1); // Garante que o tamanho não é zero
                        pht.build_table(all_records); // Assumindo que build_table lida com vetor vazio
                        std::cout << "Tabela Hash Perfeita construida com " << pht.get_count() << " registros." << std::endl;
                        menu_monitor_terremotos(pht, "Tabela Hash Perfeita", country_risks, all_records);
                        break;
                    }
                    case 0:
                        std::cout << "Retornando ao Menu Principal..." << std::endl;
                        break;
                    default:
                        std::cout << "Opcao de estrutura invalida." << std::endl;
                }
                // Pausa após sair do menu de uma estrutura (exceto se voltou com 0)
                if (ds_choice != 0) {
                    std::cout << "\nPressione Enter para retornar ao Menu Principal...";
                    std::cin.get();
                }
                break; // Fim do case 1 (Selecionar Estrutura)
            }
            case 2:
                if (all_records.empty()) {
                     std::cout << "Nao ha dados de terremotos para processar alertas." << std::endl;
                } else {
                    alerta_terremoto(country_risks, all_records);
                }
                std::cout << "\nPressione Enter para continuar...";
                std::cin.get();
                break;
            case 3:
                if (all_records.empty()) {
                     std::cout << "Nao ha dados para calcular estatisticas." << std::endl;
                } else {
                    menu_estatisticas(all_records);
                }
                 // O menu_estatisticas já deve ter sua própria pausa se necessário.
                // Se não, adicione aqui:
                // std::cout << "\nPressione Enter para continuar...";
                // std::cin.get();
                break;
            case 4:
                if (all_records.empty()) {
                     std::cout << "Nao ha dados para o modelo de previsao de tendencias." << std::endl;
                } else {
                    show_trends_menu_with_predefined_model(all_records, python_model);
                }
                 // O show_trends_menu já tem seu próprio loop e pausa
                break;
            case 5:
                if (country_risks.empty()) {
                    std::cout << "Nenhuma classificacao de risco de paises disponivel." << std::endl;
                    if (all_records.empty()){
                        std::cout << "O dataset de terremotos esta vazio." << std::endl;
                    } else {
                        std::cout << "Verifique se os dados de paises foram processados corretamente." << std::endl;
                    }
                } else {
                    menu_risco_paises(country_risks);
                }
                // O menu_risco_paises já deve ter sua própria pausa.
                // Se não, adicione aqui:
                // std::cout << "\nPressione Enter para continuar...";
                // std::cin.get();
                break;
            case 6:
                if (all_records.empty()) {
                    std::cout << "Nao e possivel executar benchmarks: o dataset de terremotos validos esta vazio." << std::endl;
                } else {
                    benchmark_menu(all_records);
                }
                // O benchmark_menu já deve ter sua própria pausa.
                break;
            case 0:
                std::cout << "Encerrando o programa..." << std::endl;
                break;
            default:
                std::cout << "Opcao invalida. Tente novamente." << std::endl;
                std::cout << "\nPressione Enter para continuar...";
                std::cin.get();
        }

    } while (main_choice != 0);

    std::cout << "\nObrigado por usar o sistema de Monitoramento de Terremotos Globais!" << std::endl;
    return 0;
}