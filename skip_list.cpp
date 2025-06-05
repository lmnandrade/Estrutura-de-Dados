#include "skip_list.h"
#include <iostream>
#include <iomanip>   // Para std::setprecision, std::fixed
#include <algorithm> // Para std::transform (se for normalizar chaves)

// Se KDTreeHelpers não for acessível por inclusão, você precisará da definição de try_string_to_double aqui.
// Assumindo que está em um header incluído (ex: kd_tree.h que é incluído por operations_menu.h)
// ou que operations_menu.h (que tem KDTreeHelpers) será incluído antes de skip_list.h em algum lugar.
// Para garantir, podemos declarar o namespace e a função aqui se não houver um utils.h
// Isso é para list_by_min_magnitude.
namespace KDTreeHelpers {
    // Esta é uma cópia da definição. Se você criar um utils.h, seria melhor colocá-la lá.
    inline bool try_string_to_double(const std::string& s, double& out, const std::string& field_name_for_error = "", bool suppress_error_message = false) {
        try {
            out = std::stod(s);
            return true;
        } catch (const std::invalid_argument& ia) {
            if (!suppress_error_message && !field_name_for_error.empty()) {
                std::cerr << "AVISO (Conversao SL): Valor invalido para " << field_name_for_error << ": '" << s << "'." << std::endl;
            } else if (!suppress_error_message) {
                 std::cerr << "AVISO (Conversao SL): Valor invalido: '" << s << "'." << std::endl;
            }
            return false;
        } catch (const std::out_of_range& oor) {
            if (!suppress_error_message && !field_name_for_error.empty()) {
                std::cerr << "AVISO (Conversao SL): Valor fora do intervalo para " << field_name_for_error << ": '" << s << "'." << std::endl;
            } else if (!suppress_error_message) {
                 std::cerr << "AVISO (Conversao SL): Valor fora do intervalo: '" << s << "'." << std::endl;
            }
            return false;
        }
    }
}

// --- Construtor ---
SkipList::SkipList() : current_max_level(0), num_elements(0), rng(std::random_device{}()), dist(0.0, 1.0) {
    // Cria o nó header. Sua chave pode ser um valor mínimo ou não usada para comparação direta.
    // O nível do header é MAX_LEVEL_SKIP_LIST.
    EarthquakeRecord dummy_header_record; // Record vazio ou com valores sentinela
    dummy_header_record.date = "HEADER"; // Apenas para identificação, não será buscado
    header = new SkipListNode(dummy_header_record, MAX_LEVEL_SKIP_LIST -1); // Níveis 0 a MAX_LEVEL-1
}

// --- Destrutor ---
SkipList::~SkipList() {
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        SkipListNode* next = current->forward[0];
        delete current;
        current = next;
    }
    delete header;
}

// --- Gerador de Nível Aleatório ---
int SkipList::random_level() {
    int lvl = 0;
    // Enquanto o número aleatório for menor que P e o nível for menor que o máximo
    while (dist(rng) < P_SKIP_LIST && lvl < MAX_LEVEL_SKIP_LIST - 1) {
        lvl++;
    }
    return lvl;
}

// --- Inserção ---
void SkipList::insert_record(const EarthquakeRecord& record) {
    std::vector<SkipListNode*> update(MAX_LEVEL_SKIP_LIST, nullptr);
    SkipListNode* current = header;
    std::string key_to_insert = SkipListNode(record, 0).get_key(); // Chave do novo nó

    // 1. Encontrar a posição de inserção e preencher o vetor 'update'
    // update[i] contém o ponteiro para o nó no nível i cujo próximo nó será o novo nó (ou já é maior)
    for (int i = current_max_level; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key_to_insert) {
            current = current->forward[i];
        }
        update[i] = current; // Armazena o nó que precede o local de inserção no nível i
    }

    // Avança para o nó que seria o próximo no nível 0
    current = current->forward[0];

    // 2. Verificar se a chave já existe (não permitir duplicatas pela chave data+hora+cidade)
    if (current != nullptr && current->get_key() == key_to_insert) {
        // std::cout << "AVISO (SkipList): Chave '" << key_to_insert << "' ja existe. Registro nao inserido." << std::endl;
        return; // Chave duplicada
    }

    // 3. Se a chave não existe, gerar um nível para o novo nó e inseri-lo
    int new_level = random_level();

    // Se o novo nível for maior que o nível máximo atual da lista,
    // atualizar o current_max_level e os ponteiros do header para os novos níveis.
    if (new_level > current_max_level) {
        for (int i = current_max_level + 1; i <= new_level; i++) {
            update[i] = header; // O header precederá o novo nó nesses níveis mais altos
        }
        current_max_level = new_level;
    }

    // Criar o novo nó
    SkipListNode* new_node = new SkipListNode(record, new_level);
    num_elements++;

    // Conectar o novo nó na lista
    for (int i = 0; i <= new_level; i++) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }
    // std::cout << "INFO (SkipList): Registro inserido com chave '" << key_to_insert << "' no nivel " << new_level << std::endl;
}

// --- Busca ---
EarthquakeRecord* SkipList::search_record(const std::string& date, const std::string& time_utc, const std::string& city)const {
    EarthquakeRecord temp_record;
    temp_record.date = date;
    temp_record.time = time_utc;
    temp_record.city = city;
    std::string key_to_find = SkipListNode(temp_record, 0).get_key();

    SkipListNode* current = header;

    // Começa do nível mais alto e desce
    for (int i = current_max_level; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key_to_find) {
            current = current->forward[i];
        }
    }

    // Após o loop, current é o nó que precede o nó procurado (ou onde ele deveria estar)
    // Move para o próximo nó no nível 0
    current = current->forward[0];

    // Verifica se o nó atual é o procurado
    if (current != nullptr && current->get_key() == key_to_find) {
        return &(current->record);
    }

    return nullptr; // Não encontrado
}

// --- Remoção ---
bool SkipList::remove_record(const std::string& date, const std::string& time_utc, const std::string& city) {
    EarthquakeRecord temp_record;
    temp_record.date = date;
    temp_record.time = time_utc;
    temp_record.city = city;
    std::string key_to_remove = SkipListNode(temp_record, 0).get_key();

    std::vector<SkipListNode*> update(MAX_LEVEL_SKIP_LIST, nullptr);
    SkipListNode* current = header;

    // 1. Encontrar o nó a ser removido e preencher 'update'
    for (int i = current_max_level; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key_to_remove) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0]; // Nó candidato à remoção

    // 2. Se o nó for encontrado
    if (current != nullptr && current->get_key() == key_to_remove) {
        // Remover o nó de todos os níveis em que aparece
        for (int i = 0; i <= current_max_level; i++) { // Iterar até o nível máximo atual da lista, não do nó
            if (update[i]->forward[i] != current) { // Se o nó não está neste nível, pare para este nível
                // Isso pode acontecer se o nó a ser removido não atingiu esse nível 'i'
                // mas o current_max_level da lista é maior.
                // No entanto, o loop sobre update[i] já deve ter posicionado corretamente.
                // Se update[i]->forward[i] é current, então ele está no caminho.
                // Esta condição (update[i]->forward[i] != current) deve ser na verdade uma verificação
                // se o current está no nível i (ou seja, se update[i]->forward[i] == current).
                // O loop correto é:
                // for (int i = 0; i <= current->forward.size() -1 ; i++) // Iterar pelos níveis do nó a ser removido
                // e usar update[i] para religar
                break; // Se o ponteiro em update[i] não aponta para 'current', terminamos para este nível
                       // Isso não deveria acontecer se a busca foi correta e current é o nó.
                       // A lógica mais simples é iterar pelos níveis do nó 'current'
            }
             if (i < current->forward.size() && update[i]->forward[i] == current) { // Verifica se o nível i é válido para current
                 update[i]->forward[i] = current->forward[i];
             }
        }
        
        delete current;
        num_elements--;

        // Ajustar current_max_level se o nível mais alto ficou vazio
        while (current_max_level > 0 && header->forward[current_max_level] == nullptr) {
            current_max_level--;
        }
        return true;
    }
    return false; // Nó não encontrado
}


// --- Funções Auxiliares e de Listagem ---
int SkipList::get_count() const {
    return num_elements;
}

bool SkipList::is_empty() const {
    return num_elements == 0;
}

void SkipList::list_all_records() const {
    if (is_empty()) {
        std::cout << "A Skip List esta vazia." << std::endl;
        return;
    }
    std::cout << "\n--- Todos os Registros de Terremotos na Skip List (" << num_elements << ") ---" << std::endl;
    SkipListNode* current = header->forward[0];
    int record_num = 1;
    while (current != nullptr) {
        std::cout << "Registro #" << record_num++ << " (Chave: "" << current->get_key() << ""):" << std::endl;
        std::cout << "  ";
        displayRecord(current->record); // Função de csv_reader.h
        if (num_elements > 1 && current->forward[0] != nullptr) {
            std::cout << "  --------------------------------------------------------------------------------" << std::endl;
        }
        current = current->forward[0];
    }
    if (num_elements > 0) {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}

std::vector<EarthquakeRecord> SkipList::get_all_records_vector() const {
    std::vector<EarthquakeRecord> records;
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        records.push_back(current->record);
        current = current->forward[0];
    }
    return records;
}

void SkipList::list_by_min_magnitude(const std::string& min_mag_str) const {
    if (is_empty()) {
        std::cout << "A Skip List esta vazia. Nada para listar." << std::endl;
        return;
    }
    double min_mag_val;
    if (!KDTreeHelpers::try_string_to_double(min_mag_str, min_mag_val, "Magnitude Minima", false)) {
        return;
    }
    std::cout << "\n--- Registros na Skip List com Magnitude >= " << std::fixed << std::setprecision(1) << min_mag_val << " ---" << std::endl;
    int total_found = 0;
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        double current_rec_mag;
        if (KDTreeHelpers::try_string_to_double(current->record.magnitude, current_rec_mag, "", true)) {
            if (current_rec_mag >= min_mag_val) {
                total_found++;
                std::cout << "Registro Filtrado #" << total_found << " (Chave: "" << current->get_key() << ""):" << std::endl;
                std::cout << "  "; displayRecord(current->record);
                if (current->forward[0] != nullptr) { // Evita linha extra se for o último
                     // std::cout << "  --------------------------------------------------------------------------------" << std::endl;
                     // ^^^ Linha separadora pode ser muita poluição visual aqui, melhor só uma no final.
                }
            }
        }
        current = current->forward[0];
    }
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado com magnitude >= " << min_mag_str << "." << std::endl;
    } else {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}

void SkipList::list_by_year(const std::string& year_str) const {
    if (is_empty()) {
        std::cout << "A Skip List esta vazia. Nada para listar." << std::endl;
        return;
    }
    if (year_str.length() != 4) {
        std::cerr << "Erro: Formato de ano invalido. Use AAAA (ex: 2023)." << std::endl;
        return;
    }
    try { std::stoi(year_str); }
    catch (const std::exception&) {
        std::cerr << "Erro: Ano invalido: "" << year_str << """ << std::endl;
        return;
    }

    std::cout << "\n--- Registros na Skip List do Ano " << year_str << " ---" << std::endl;
    int total_found = 0;
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        if (current->record.date.rfind(year_str, 0) == 0) { // Checa se a data começa com o ano
            total_found++;
            std::cout << "Registro Filtrado #" << total_found << " (Chave: "" << current->get_key() << ""):" << std::endl;
            std::cout << "  "; displayRecord(current->record);
        }
        current = current->forward[0];
    }
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado para o ano " << year_str << "." << std::endl;
    } else {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}

// --- Outras buscas (serão varreduras, pois a Skip List é otimizada pela chave primária) ---
std::vector<EarthquakeRecord> SkipList::search_by_city(const std::string& city) const {
    std::vector<EarthquakeRecord> results;
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        if (current->record.city == city) {
            results.push_back(current->record);
        }
        current = current->forward[0];
    }
    return results;
}

std::vector<EarthquakeRecord> SkipList::search_by_magnitude_exact(const std::string& magnitude) const {
    std::vector<EarthquakeRecord> results;
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        if (current->record.magnitude == magnitude) {
            results.push_back(current->record);
        }
        current = current->forward[0];
    }
    return results;
}

std::vector<EarthquakeRecord> SkipList::search_by_date(const std::string& date) const {
    std::vector<EarthquakeRecord> results;
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        if (current->record.date == date) {
            results.push_back(current->record);
        }
        current = current->forward[0];
    }
    return results;
}

std::vector<EarthquakeRecord> SkipList::search_by_city_and_magnitude(const std::string& city, const std::string& magnitude) const {
    std::vector<EarthquakeRecord> results;
    SkipListNode* current = header->forward[0];
    while (current != nullptr) {
        if (current->record.city == city && current->record.magnitude == magnitude) {
            results.push_back(current->record);
        }
        current = current->forward[0];
    }
    return results;
}

// Implementação para a assinatura de remoção com 4 parâmetros (para consistência com operations_menu)
// Esta será uma busca pela chave primária (date, time, city) e depois verifica o país.
bool SkipList::remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country) {
    EarthquakeRecord temp_record;
    temp_record.date = date;
    temp_record.time = time_utc;
    temp_record.city = city;
    // O país não faz parte da chave primária da Skip List, mas podemos usá-lo para confirmar
    // o registro a ser removido se a chave (data, hora, cidade) não for suficiente para unicidade
    // ou se a política de remoção exigir uma correspondência exata incluindo o país.

    std::string key_to_remove = SkipListNode(temp_record, 0).get_key();

    std::vector<SkipListNode*> update(MAX_LEVEL_SKIP_LIST, nullptr);
    SkipListNode* current = header;

    for (int i = current_max_level; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->get_key() < key_to_remove) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];

    if (current != nullptr && current->get_key() == key_to_remove) {
        // Chave primária encontrada. Agora, verificamos o país para garantir que é o registro correto.
        // Se a política for remover qualquer um com a chave primária, esta verificação de país pode ser opcional.
        // Para consistência com a remoção da lista raw, é bom verificar o país.
        if (current->record.country == country) {
            for (int i = 0; i <= current_max_level; i++) {
                 if (i < current->forward.size() && update[i]->forward[i] == current) {
                    update[i]->forward[i] = current->forward[i];
                } else if (i >= current->forward.size() && update[i]->forward[i] == current){
                    // Isso pode ocorrer se o current_max_level for maior que o nível real do nó.
                    // Neste caso, o header deve apontar para o próximo do current.
                    // A lógica principal já deve cobrir isso.
                    // O importante é que update[i]->forward[i] seja religado.
                }
            }
            delete current;
            num_elements--;

            while (current_max_level > 0 && header->forward[current_max_level] == nullptr) {
                current_max_level--;
            }
            return true;
        } else {
            // Chave primária (data, hora, cidade) corresponde, mas o país não.
            // std::cout << "AVISO (SkipList Remove): Chave " << key_to_remove << " encontrada, mas o pais nao corresponde ("
            //           << current->record.country << " vs " << country << "). Registro nao removido." << std::endl;
            return false;
        }
    }
    return false; // Nó não encontrado com a chave primária
}
