#include "lista_dupla_ligada.h" // Certifique-se que csv_reader.h está incluído em lista_dupla_ligada.h ou que EarthquakeRecord é conhecido
#include <stdexcept> // Para std::stod, std::invalid_argument, std::out_of_range
#include <iomanip> // Para std::setw, std::left (na formatação da listagem)
#include <iostream> // Para std::cout, std::cerr

// --- Implementação da Classe DoublyLinkedList ---

DoublyLinkedList::DoublyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

DoublyLinkedList::~DoublyLinkedList() {
clear_list();
}

void DoublyLinkedList::clear_list() {
Node* current = head;
while (current != nullptr) {
Node* next_node = current->next;
delete current;
current = next_node;
}
head = nullptr;
tail = nullptr;
count = 0;
}

bool DoublyLinkedList::is_empty() const {
return head == nullptr;
}

int DoublyLinkedList::get_count() const {
return count;
}

// Função privada para exibir um registro (usada internamente por funções de listagem)
void DoublyLinkedList::display_record_details(const EarthquakeRecord& record) const {
// Usando a função global displayRecord de csv_reader.h para consistência,
// ou você pode manter uma formatação específica aqui.
// Se displayRecord de csv_reader.h já faz o que você quer:
std::cout << " "; // Pequena indentação
displayRecord(record); // Chamando a função global (precisa estar declarada em csv_reader.h e incluída)

// Ou, se quiser uma formatação específica para a lista:
/*
std::cout << "  Date: " << std::setw(12) << std::left << record.date
          << "Time (UTC): " << std::setw(10) << std::left << record.time // Assumindo que o campo é 'time'
          << "Magnitude: " << std::setw(5) << std::left << record.magnitude
          << "Depth: " << std::setw(7) << std::left << record.depth + " km" // Assumindo que o campo é 'depth'
          << "Impact: " << std::setw(5) << std::left << record.impact_score << std::endl;
std::cout << "  Location: " << record.city << ", " << record.country
          << " (Lat: " << record.latitude << ", Lon: " << record.longitude << ")" << std::endl;
std::cout << "  --------------------------------------------------------------------------------" << std::endl;
*/
}

// 1. Adicionar novo registro (insere no final)
void DoublyLinkedList::insert_record(const EarthquakeRecord& record) {
Node* new_node = new Node(record);
if (is_empty()) {
head = new_node;
tail = new_node;
} else {
tail->next = new_node;
new_node->prev = tail;
tail = new_node;
}
count++;
}

// 2. Remover registro
// Remove o primeiro registro que corresponder aos critérios de identificação.
bool DoublyLinkedList::remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country) {
    if (is_empty()) {
        return false;
    }

    Node* current = head;
    while (current != nullptr) {
        if (current->data.date == date &&
            current->data.time == time_utc &&
            current->data.city == city &&
            current->data.country == country) {

            // Remoção do nó
            if (current == head) {
                head = current->next;
                if (head != nullptr) {
                    head->prev = nullptr;
                } else {
                    tail = nullptr;
                }
            } else if (current == tail) {
                tail = current->prev;
                tail->next = nullptr;
            } else {
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }
            delete current;
            count--;
            return true;
        }
        current = current->next;
    }
    return false;
}

// 3. Funções de Busca
std::vector<EarthquakeRecord> DoublyLinkedList::search_by_city(const std::string& city_name) const {
std::vector<EarthquakeRecord> results;
Node* current = head;
while (current != nullptr) {
if (current->data.city == city_name) {
results.push_back(current->data);
}
current = current->next;
}
return results;
}

std::vector<EarthquakeRecord> DoublyLinkedList::search_by_magnitude_exact(const std::string& magnitude_value) const {
std::vector<EarthquakeRecord> results;
Node* current = head;
while (current != nullptr) {
if (current->data.magnitude == magnitude_value) {
results.push_back(current->data);
}
current = current->next;
}
return results;
}

std::vector<EarthquakeRecord> DoublyLinkedList::search_by_date(const std::string& date_value) const {
std::vector<EarthquakeRecord> results;
Node* current = head;
while (current != nullptr) {
if (current->data.date == date_value) {
results.push_back(current->data);
}
current = current->next;
}
return results;
}

std::vector<EarthquakeRecord> DoublyLinkedList::search_by_city_and_magnitude(const std::string& city_name, const std::string& magnitude_value) const {
std::vector<EarthquakeRecord> results;
Node* current = head;
while (current != nullptr) {
if (current->data.city == city_name && current->data.magnitude == magnitude_value) {
results.push_back(current->data);
}
current = current->next;
}
return results;
}

// 4. Funções de Listagem
void DoublyLinkedList::list_all_records() const {
if (is_empty()) {
std::cout << "A lista de registros de terremotos esta vazia." << std::endl;
return;
}
std::cout << "\n--- Todos os Registros de Terremotos na Lista (" << count << ") ---" << std::endl;
Node* current = head;
int record_num = 1;
while (current != nullptr) {
std::cout << "Registro #" << record_num++ << ":" << std::endl;
display_record_details(current->data); // Usa a função privada ou a global
if (record_num > 1) std::cout << " --------------------------------------------------------------------------------" << std::endl; // Para separar melhor
current = current->next;
}
}

void DoublyLinkedList::list_by_min_magnitude(const std::string& min_mag_str) const {
if (is_empty()) {
std::cout << "A lista esta vazia. Nada para listar." << std::endl;
return;
}

double min_magnitude_val;
try {
    min_magnitude_val = std::stod(min_mag_str);
} catch (const std::invalid_argument& ia) {
    std::cerr << "Erro: Valor de magnitude minima invalido: " << min_mag_str << std::endl;
    return;
} catch (const std::out_of_range& oor) {
    std::cerr << "Erro: Valor de magnitude minima fora do intervalo: " << min_mag_str << std::endl;
    return;
}

std::cout << "\n--- Registros com Magnitude >= " << min_mag_str << " ---" << std::endl;
Node* current = head;
int found_count = 0;
int record_num = 1;
while (current != nullptr) {
    try {
        double current_mag = std::stod(current->data.magnitude);
        if (current_mag >= min_magnitude_val) {
            std::cout << "Registro Filtrado #" << record_num++ << ":" << std::endl;
            display_record_details(current->data);
            if (found_count > 0) std::cout << "  --------------------------------------------------------------------------------" << std::endl;
            found_count++;
        }
    } catch (const std::exception& e) {
        // Ignorar silenciosamente ou logar se um registro tiver magnitude mal formatada
    }
    current = current->next;
}
if (found_count == 0) {
    std::cout << "Nenhum registro encontrado com magnitude >= " << min_mag_str << "." << std::endl;
}
}

void DoublyLinkedList::list_by_year(const std::string& year_str) const {
if (is_empty()) {
std::cout << "A lista esta vazia. Nada para listar." << std::endl;
return;
}
if (year_str.length() != 4) {
std::cerr << "Erro: Formato de ano invalido. Use AAAA (ex: 2023)." << std::endl;
return;
}
try {
std::stoi(year_str);
} catch (const std::exception& e) {
std::cerr << "Erro: Ano invalido: " << year_str << std::endl;
return;
}

std::cout << "\n--- Registros do Ano " << year_str << " ---" << std::endl;
Node* current = head;
int found_count = 0;
int record_num = 1;
while (current != nullptr) {
    if (current->data.date.rfind(year_str, 0) == 0) {
        std::cout << "Registro Filtrado #" << record_num++ << ":" << std::endl;
        display_record_details(current->data);
        if (found_count > 0) std::cout << "  --------------------------------------------------------------------------------" << std::endl;
        found_count++;
    }
    current = current->next;
}
if (found_count == 0) {
    std::cout << "Nenhum registro encontrado para o ano " << year_str << "." << std::endl;
}
}

// Função para retornar todos os registros como um vetor
// Útil para operações externas que precisam de todos os dados (ex: reclassificação)
std::vector<EarthquakeRecord> DoublyLinkedList::get_all_records_vector() const {
std::vector<EarthquakeRecord> all_records;
Node* current = head;
while (current != nullptr) {
all_records.push_back(current->data);
current = current->next;
}
return all_records;
}