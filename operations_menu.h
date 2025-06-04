#ifndef OPERATIONS_MENU_H
#define OPERATIONS_MENU_H

#include "csv_reader.h"      // Para EarthquakeRecord
#include "lista_dupla_ligada.h" // Ou outras estruturas, se o menu for genérico
#include "classification.h"  // Para CountryRiskProfile
#include <vector>
#include <string>
#include <map>

// Função principal para o menu "Monitorar Terremotos"
// Recebe a estrutura de dados já populada e o mapa de perfis de risco.
// A estrutura de dados é passada por referência para permitir modificações (inserir/remover).
void menu_monitor_terremotos(
    DoublyLinkedList& data_structure, // Pode ser um template para outras estruturas
    const std::map<std::string, CountryRiskProfile>& country_risks,
    const std::vector<EarthquakeRecord>& all_raw_records // Para reclassificar se necessário
);

// Função principal para o menu "Alerta de Terremoto"
// Recebe os perfis de risco dos países e talvez uma lista de emails cadastrados.
void menu_alerta_terremoto(
    const std::map<std::string, CountryRiskProfile>& country_risks,
    const std::vector<std::string>& registered_emails, // Lista de emails cadastrados
    const std::vector<EarthquakeRecord>& all_raw_records // Para verificar se o país já apareceu
);

void menu_monitor_terremotos(
    DoublyLinkedList& data_structure,
    const std::map<std::string, CountryRiskProfile>& country_risks, // Perfis são para consulta, podem ser const
    std::vector<EarthquakeRecord>& all_raw_records_ref // MODIFICADO: Passado por referência não-const
);


#endif // OPERATIONS_MENU_H