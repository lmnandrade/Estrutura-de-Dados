#include "avl_tree.h"
#include <iostream>
#include <iomanip> // For list formatting

// --- Private Helper Methods (existing) ---
std::string AVLTree::get_composite_key(const EarthquakeRecord& rec) const {
    return rec.date + "_" + rec.time + "_" + rec.city + "_" + rec.country;
}

std::string AVLTree::get_composite_key(const std::string& date, const std::string& time,
                                       const std::string& city, const std::string& country) const {
    return date + "_" + time + "_" + city + "_" + country;
}

int AVLTree::height(AVLNode* N) {
    if (N == nullptr) return 0;
    return N->height;
}

int AVLTree::get_balance(AVLNode* N) {
    if (N == nullptr) return 0;
    return height(N->left) - height(N->right);
}

AVLNode* AVLTree::right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;
    return x;
}

AVLNode* AVLTree::left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;
    return y;
}

AVLNode* AVLTree::insert_node_recursive(AVLNode* node, const EarthquakeRecord& rec, const std::string& key) {
    if (node == nullptr) {
        node_count++;
        return new AVLNode(rec);
    }
    std::string node_key = get_composite_key(node->record);
    if (key < node_key)
        node->left = insert_node_recursive(node->left, rec, key);
    else if (key > node_key)
        node->right = insert_node_recursive(node->right, rec, key);
    else {
        return node; // Duplicate keys not inserted
    }

    node->height = 1 + std::max(height(node->left), height(node->right));
    int balance = get_balance(node);

    if (balance > 1 && key < get_composite_key(node->left->record))
        return right_rotate(node);
    if (balance < -1 && key > get_composite_key(node->right->record))
        return left_rotate(node);
    if (balance > 1 && key > get_composite_key(node->left->record)) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }
    if (balance < -1 && key < get_composite_key(node->right->record)) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }
    return node;
}

AVLNode* AVLTree::find_min_value_node(AVLNode* node) {
    AVLNode* current = node;
    while (current && current->left != nullptr)
        current = current->left;
    return current;
}

AVLNode* AVLTree::remove_node_recursive(AVLNode* current_node, const std::string& key_to_delete) {
    if (current_node == nullptr) return current_node;
    std::string current_node_key = get_composite_key(current_node->record);

    if (key_to_delete < current_node_key)
        current_node->left = remove_node_recursive(current_node->left, key_to_delete);
    else if (key_to_delete > current_node_key)
        current_node->right = remove_node_recursive(current_node->right, key_to_delete);
    else {
        if ((current_node->left == nullptr) || (current_node->right == nullptr)) {
            AVLNode* temp = current_node->left ? current_node->left : current_node->right;
            if (temp == nullptr) {
                temp = current_node;
                current_node = nullptr;
            } else {
                *current_node = *temp;
            }
            delete temp;
            node_count--;
        } else {
            AVLNode* temp = find_min_value_node(current_node->right);
            current_node->record = temp->record;
            current_node->right = remove_node_recursive(current_node->right, get_composite_key(temp->record));
        }
    }

    if (current_node == nullptr) return current_node;

    current_node->height = 1 + std::max(height(current_node->left), height(current_node->right));
    int balance = get_balance(current_node);

    if (balance > 1 && get_balance(current_node->left) >= 0)
        return right_rotate(current_node);
    if (balance > 1 && get_balance(current_node->left) < 0) {
        current_node->left = left_rotate(current_node->left);
        return right_rotate(current_node);
    }
    if (balance < -1 && get_balance(current_node->right) <= 0)
        return left_rotate(current_node);
    if (balance < -1 && get_balance(current_node->right) > 0) {
        current_node->right = right_rotate(current_node->right);
        return left_rotate(current_node);
    }
    return current_node;
}

AVLNode* AVLTree::search_node_recursive(AVLNode* node, const std::string& key) const {
    if (node == nullptr) return nullptr;
    std::string node_key = get_composite_key(node->record);
    if (key == node_key) return node;
    if (key < node_key) return search_node_recursive(node->left, key);
    else return search_node_recursive(node->right, key);
}

void AVLTree::get_all_records_recursive(AVLNode* node, std::vector<EarthquakeRecord>& result_vector) const {
    if (node != nullptr) {
        get_all_records_recursive(node->left, result_vector);
        result_vector.push_back(node->record);
        get_all_records_recursive(node->right, result_vector);
    }
}

void AVLTree::destroy_recursive(AVLNode* node) {
    if (node != nullptr) {
        destroy_recursive(node->left);
        destroy_recursive(node->right);
        delete node;
    }
}

// --- Public Methods (existing) ---
AVLTree::AVLTree() : root(nullptr), node_count(0) {}

AVLTree::~AVLTree() {
    destroy_recursive(root);
}

void AVLTree::clear_tree() {
    destroy_recursive(root);
    root = nullptr;
    node_count = 0;
}

void AVLTree::insert_record(const EarthquakeRecord& record) {
    std::string key = get_composite_key(record);
    if (search_node_recursive(root, key) == nullptr) { // Evita duplicatas e contagem dupla
         root = insert_node_recursive(root, record, key);
    } else {
        // std::cout << "AVISO (AVL): Chave duplicada '" << key << "' nao inserida." << std::endl;
    }
}

bool AVLTree::remove_record(const std::string& date, const std::string& time,
                            const std::string& city, const std::string& country) {
    std::string key_to_delete = get_composite_key(date, time, city, country);
    int initial_count = node_count;
    root = remove_node_recursive(root, key_to_delete);
    return node_count < initial_count;
}

const EarthquakeRecord* AVLTree::search_record_by_composite_key(const std::string& date, const std::string& time,
                                             const std::string& city, const std::string& country) const {
    std::string key_to_search = get_composite_key(date, time, city, country);
    AVLNode* found_node = search_node_recursive(root, key_to_search);
    if (found_node != nullptr) {
        return &(found_node->record);
    }
    return nullptr;
}

std::vector<EarthquakeRecord> AVLTree::get_all_records() const {
    std::vector<EarthquakeRecord> result_vector;
    get_all_records_recursive(root, result_vector);
    return result_vector;
}

bool AVLTree::is_empty() const {
    return root == nullptr;
}

int AVLTree::get_count() const {
    return node_count;
}


// --- Implementação dos Novos Métodos da Interface Comum ---

// Helpers recursivos para buscas que exigem varredura (não pela chave primária da AVL)
void AVLTree::search_by_city_recursive(AVLNode* node, const std::string& city_name, std::vector<EarthquakeRecord>& results) const {
    if (node == nullptr) return;
    search_by_city_recursive(node->left, city_name, results);
    if (node->record.city == city_name) {
        results.push_back(node->record);
    }
    search_by_city_recursive(node->right, city_name, results);
}

std::vector<EarthquakeRecord> AVLTree::search_by_city(const std::string& city_name) const {
    std::vector<EarthquakeRecord> results;
    search_by_city_recursive(root, city_name, results);
    return results;
}

void AVLTree::search_by_magnitude_exact_recursive(AVLNode* node, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const {
    if (node == nullptr) return;
    search_by_magnitude_exact_recursive(node->left, magnitude_value, results);
    if (node->record.magnitude == magnitude_value) {
        results.push_back(node->record);
    }
    search_by_magnitude_exact_recursive(node->right, magnitude_value, results);
}

std::vector<EarthquakeRecord> AVLTree::search_by_magnitude_exact(const std::string& magnitude_value) const {
    std::vector<EarthquakeRecord> results;
    search_by_magnitude_exact_recursive(root, magnitude_value, results);
    return results;
}

void AVLTree::search_by_date_recursive(AVLNode* node, const std::string& date_value, std::vector<EarthquakeRecord>& results) const {
    if (node == nullptr) return;
    search_by_date_recursive(node->left, date_value, results);
    if (node->record.date == date_value) {
        results.push_back(node->record);
    }
    search_by_date_recursive(node->right, date_value, results);
}

std::vector<EarthquakeRecord> AVLTree::search_by_date(const std::string& date_value) const {
    std::vector<EarthquakeRecord> results;
    search_by_date_recursive(root, date_value, results);
    return results;
}

void AVLTree::search_by_city_and_magnitude_recursive(AVLNode* node, const std::string& city_name, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const {
    if (node == nullptr) return;
    search_by_city_and_magnitude_recursive(node->left, city_name, magnitude_value, results);
    if (node->record.city == city_name && node->record.magnitude == magnitude_value) {
        results.push_back(node->record);
    }
    search_by_city_and_magnitude_recursive(node->right, city_name, magnitude_value, results);
}

std::vector<EarthquakeRecord> AVLTree::search_by_city_and_magnitude(const std::string& city_name, const std::string& magnitude_value) const {
    std::vector<EarthquakeRecord> results;
    search_by_city_and_magnitude_recursive(root, city_name, magnitude_value, results);
    return results;
}

// Helpers recursivos para listagem
void AVLTree::list_all_records_recursive(AVLNode* node, int& record_num) const {
    if (node == nullptr) return;
    list_all_records_recursive(node->left, record_num);
    std::cout << "Registro #" << record_num++ << ":" << std::endl;
    displayRecord(node->record); // displayRecord é de csv_reader.h
    std::cout << "  --------------------------------------------------------------------------------" << std::endl;
    list_all_records_recursive(node->right, record_num);
}

void AVLTree::list_all_records() const {
    if (is_empty()) {
        std::cout << "A arvore AVL esta vazia." << std::endl;
        return;
    }
    std::cout << "\n--- Todos os Registros de Terremotos na Arvore AVL (" << node_count << ") ---" << std::endl;
    int record_num = 1;
    list_all_records_recursive(root, record_num);
}

void AVLTree::list_by_min_magnitude_recursive(AVLNode* node, double min_mag_val, int& record_num, int& found_count) const {
    if (node == nullptr) return;
    list_by_min_magnitude_recursive(node->left, min_mag_val, record_num, found_count);
    try {
        double current_mag = std::stod(node->record.magnitude);
        if (current_mag >= min_mag_val) {
            std::cout << "Registro Filtrado #" << record_num++ << ":" << std::endl;
            displayRecord(node->record);
            std::cout << "  --------------------------------------------------------------------------------" << std::endl;
            found_count++;
        }
    } catch (const std::exception&) { /* Ignorar registros com magnitude mal formatada */ }
    list_by_min_magnitude_recursive(node->right, min_mag_val, record_num, found_count);
}

void AVLTree::list_by_min_magnitude(const std::string& min_mag_str) const {
    if (is_empty()) {
        std::cout << "A arvore esta vazia. Nada para listar." << std::endl;
        return;
    }
    double min_mag_val;
    try {
        min_mag_val = std::stod(min_mag_str);
    } catch (const std::exception& e) {
        std::cerr << "Erro: Valor de magnitude minima invalido: " << min_mag_str << " (" << e.what() << ")" << std::endl;
        return;
    }
    std::cout << "\n--- Registros na AVL com Magnitude >= " << min_mag_str << " ---" << std::endl;
    int record_num = 1;
    int found_count = 0;
    list_by_min_magnitude_recursive(root, min_mag_val, record_num, found_count);
    if (found_count == 0) {
        std::cout << "Nenhum registro encontrado com magnitude >= " << min_mag_str << "." << std::endl;
    }
}

void AVLTree::list_by_year_recursive(AVLNode* node, const std::string& year_str, int& record_num, int& found_count) const {
    if (node == nullptr) return;
    list_by_year_recursive(node->left, year_str, record_num, found_count);
    if (node->record.date.rfind(year_str, 0) == 0) { // Verifica se a data começa com o ano
        std::cout << "Registro Filtrado #" << record_num++ << ":" << std::endl;
        displayRecord(node->record);
        std::cout << "  --------------------------------------------------------------------------------" << std::endl;
        found_count++;
    }
    list_by_year_recursive(node->right, year_str, record_num, found_count);
}

void AVLTree::list_by_year(const std::string& year_str) const {
    if (is_empty()) {
        std::cout << "A arvore esta vazia. Nada para listar." << std::endl;
        return;
    }
    if (year_str.length() != 4) {
        std::cerr << "Erro: Formato de ano invalido. Use AAAA (ex: 2023)." << std::endl;
        return;
    }
    try { std::stoi(year_str); }
    catch (const std::exception&) {
        std::cerr << "Erro: Ano invalido: " << year_str << std::endl;
        return;
    }

    std::cout << "\n--- Registros na AVL do Ano " << year_str << " ---" << std::endl;
    int record_num = 1;
    int found_count = 0;
    list_by_year_recursive(root, year_str, record_num, found_count);
    if (found_count == 0) {
        std::cout << "Nenhum registro encontrado para o ano " << year_str << "." << std::endl;
    }
}