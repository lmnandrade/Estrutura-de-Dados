#include "kd_tree.h"
#include <iomanip>      // Para formatação de saída em listagens
#include <cmath>        // Para std::fabs, std::pow
#include <limits>       // Para std::numeric_limits
#include <stdexcept>    // Para std::stod e exceções (se KDTreeHelpers não for usado)
#include <algorithm>    // Para std::reverse (necessário se a ordem da priority_queue não for a final)
#include <vector>
#include <queue>        // Para std::priority_queue
#include <iostream>     // Para std::cerr, std::cout

// --- Namespace KDTreeHelpers (Exemplo de implementação se não existir) ---
// Se você já tem KDTreeHelpers definido, pode ignorar esta seção.
// Caso contrário, esta é uma sugestão.
namespace KDTreeHelpers {
    // Tenta converter string para double. Retorna true se bem-sucedido.
    // Imprime erro se field_name_for_error não for vazio e suppress_error for false.
    inline bool try_string_to_double(const std::string& s, double& out, const std::string& field_name_for_error = "", bool suppress_error_message = false) {
        try {
            out = std::stod(s);
            return true;
        } catch (const std::invalid_argument& ia) {
            if (!suppress_error_message && !field_name_for_error.empty()) {
                std::cerr << "ERRO: Valor invalido para " << field_name_for_error << ": '" << s << "' (nao e um numero)." << std::endl;
            } else if (!suppress_error_message) {
                 std::cerr << "ERRO: Valor invalido: '" << s << "' (nao e um numero)." << std::endl;
            }
            return false;
        } catch (const std::out_of_range& oor) {
            if (!suppress_error_message && !field_name_for_error.empty()) {
                std::cerr << "ERRO: Valor fora do intervalo para " << field_name_for_error << ": '" << s << "'." << std::endl;
            } else if (!suppress_error_message) {
                 std::cerr << "ERRO: Valor fora do intervalo: '" << s << "'." << std::endl;
            }
            return false;
        }
    }

    // Compara dois EarthquakeRecords para ver se são idênticos
    // Ajuste os campos conforme a definição de EarthquakeRecord
    inline bool are_records_identical(const EarthquakeRecord& r1, const EarthquakeRecord& r2) {
        return r1.date == r2.date &&
               r1.time == r2.time &&
               r1.latitude == r2.latitude &&    // Comparando as strings originais
               r1.longitude == r2.longitude &&  // Comparando as strings originais
               r1.country == r2.country &&
               r1.city == r2.city &&
               r1.depth == r2.depth &&
               r1.magnitude == r2.magnitude &&
               r1.impact_score == r2.impact_score;
        // Adicione outros campos de EarthquakeRecord se houver.
    }
} // namespace KDTreeHelpers


// --- Construtor, Destrutor e Limpeza ---
KDTree::KDTree() : root(nullptr), node_count(0) {}

KDTree::~KDTree() {
    clear_tree();
}

void KDTree::clear_tree() {
    destroy_recursive(root);
    root = nullptr;
    node_count = 0;
}

void KDTree::destroy_recursive(KDNode* current_node) {
    if (current_node) {
        destroy_recursive(current_node->left);
        destroy_recursive(current_node->right);
        delete current_node;
    }
}

bool KDTree::is_empty() const {
    return root == nullptr;
}

int KDTree::get_count() const {
    return node_count;
}

// --- Inserção ---
KDNode* KDTree::insert_recursive(KDNode* current_node, const EarthquakeRecord& rec, double lat, double lon, unsigned depth) {
    if (current_node == nullptr) {
        node_count++;
        return new KDNode(rec, lat, lon);
    }

    unsigned current_axis = depth % K_DIMENSIONS; // K_DIMENSIONS deve ser definido (ex: 2)
    double new_point_coord = (current_axis == 0) ? lat : lon;
    double current_node_coord = current_node->point[current_axis];

    if (new_point_coord < current_node_coord) {
        current_node->left = insert_recursive(current_node->left, rec, lat, lon, depth + 1);
    } else {
        current_node->right = insert_recursive(current_node->right, rec, lat, lon, depth + 1);
    }
    return current_node;
}

void KDTree::insert_record(const EarthquakeRecord& record) {
    double lat, lon;
    // Usando o helper para converter. Se falhar, imprime aviso e não insere.
    // O terceiro argumento true para try_string_to_double suprime a mensagem de erro do helper
    // porque nós imprimimos uma mensagem customizada aqui.
    if (!KDTreeHelpers::try_string_to_double(record.latitude, lat, "Latitude", true) ||
        !KDTreeHelpers::try_string_to_double(record.longitude, lon, "Longitude", true)) {
        std::cerr << "AVISO (KDTree Insercao): Nao foi possivel converter coordenadas para o registro: ";
        displayRecord(record); // Supondo que displayRecord exista e esteja acessível
        std::cerr << "Registro nao inserido na KD-Tree." << std::endl;
        return;
    }
    root = insert_recursive(root, record, lat, lon, 0);
}

// --- Remoção ---
bool KDTree::find_exact_record_recursive(KDNode* current_node,
                                         const std::string& date, const std::string& time_utc,
                                         const std::string& city, const std::string& country,
                                         EarthquakeRecord& found_record) const {
    if (!current_node) {
        return false;
    }
    if (current_node->record.date == date &&
        current_node->record.time == time_utc &&
        current_node->record.city == city &&
        current_node->record.country == country) {
        found_record = current_node->record;
        return true;
    }
    if (find_exact_record_recursive(current_node->left, date, time_utc, city, country, found_record)) {
        return true;
    }
    return find_exact_record_recursive(current_node->right, date, time_utc, city, country, found_record);
}

KDNode* KDTree::find_min_recursive(KDNode* current_node, int target_axis, unsigned depth) {
    if (!current_node) {
        return nullptr;
    }

    unsigned current_splitting_axis = depth % K_DIMENSIONS;

    if (current_splitting_axis == target_axis) {
        if (current_node->left == nullptr) {
            return current_node;
        }
        return find_min_recursive(current_node->left, target_axis, depth + 1);
    } else {
        KDNode* left_min = find_min_recursive(current_node->left, target_axis, depth + 1);
        KDNode* right_min = find_min_recursive(current_node->right, target_axis, depth + 1);
        KDNode* res = current_node;

        if (left_min && left_min->point[target_axis] < res->point[target_axis]) {
            res = left_min;
        }
        if (right_min && right_min->point[target_axis] < res->point[target_axis]) {
            res = right_min;
        }
        return res;
    }
}

KDNode* KDTree::remove_by_exact_record_recursive(KDNode* current_node,
                                               const EarthquakeRecord& record_to_delete,
                                               double lat_del, double lon_del, unsigned depth) {
    if (current_node == nullptr) {
        return nullptr;
    }

    unsigned current_axis = depth % K_DIMENSIONS;
    double point_to_delete_coord = (current_axis == 0) ? lat_del : lon_del;
    double current_node_coord = current_node->point[current_axis];

    const double EPSILON = 1e-9;
    bool coords_match = (std::fabs(current_node->point[0] - lat_del) < EPSILON &&
                         std::fabs(current_node->point[1] - lon_del) < EPSILON);

    if (coords_match && KDTreeHelpers::are_records_identical(current_node->record, record_to_delete)) {
        if (current_node->right != nullptr) {
            KDNode* min_node = find_min_recursive(current_node->right, current_axis, depth + 1);
            current_node->record = min_node->record;
            current_node->point[0] = min_node->point[0];
            current_node->point[1] = min_node->point[1];
            current_node->right = remove_by_exact_record_recursive(current_node->right, min_node->record, min_node->point[0], min_node->point[1], depth + 1);
        } else if (current_node->left != nullptr) {
            KDNode* temp = current_node->left;
            delete current_node;
            node_count--;
            return temp;
        } else {
            delete current_node;
            node_count--;
            return nullptr;
        }
    } else {
        if (point_to_delete_coord < current_node_coord) {
            current_node->left = remove_by_exact_record_recursive(current_node->left, record_to_delete, lat_del, lon_del, depth + 1);
        } else {
            current_node->right = remove_by_exact_record_recursive(current_node->right, record_to_delete, lat_del, lon_del, depth + 1);
        }
    }
    return current_node;
}

bool KDTree::remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country) {
    if (is_empty()) return false;

    EarthquakeRecord record_to_delete;
    if (!find_exact_record_recursive(root, date, time_utc, city, country, record_to_delete)) {
        return false;
    }

    double lat_del, lon_del;
    if (!KDTreeHelpers::try_string_to_double(record_to_delete.latitude, lat_del, "Latitude (remocao)", true) ||
        !KDTreeHelpers::try_string_to_double(record_to_delete.longitude, lon_del, "Longitude (remocao)", true)) {
        std::cerr << "ERRO (KDTree Remocao): Nao foi possivel converter coordenadas do registro encontrado para remocao: ";
        displayRecord(record_to_delete);
        return false;
    }
    
    int initial_count = node_count;
    root = remove_by_exact_record_recursive(root, record_to_delete, lat_del, lon_del, 0);
    
    return node_count < initial_count;
}

// --- Funções de Busca (varredura completa, pois a KD-Tree é para busca espacial por esses atributos) ---
void KDTree::search_by_city_recursive(KDNode* current_node, const std::string& city_name, std::vector<EarthquakeRecord>& results) const {
    if (!current_node) return;
    if (current_node->record.city == city_name) {
        results.push_back(current_node->record);
    }
    search_by_city_recursive(current_node->left, city_name, results);
    search_by_city_recursive(current_node->right, city_name, results);
}
std::vector<EarthquakeRecord> KDTree::search_by_city(const std::string& city_name) const {
    std::vector<EarthquakeRecord> results;
    search_by_city_recursive(root, city_name, results);
    return results;
}

void KDTree::search_by_magnitude_exact_recursive(KDNode* current_node, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const {
    if (!current_node) return;
    if (current_node->record.magnitude == magnitude_value) {
        results.push_back(current_node->record);
    }
    search_by_magnitude_exact_recursive(current_node->left, magnitude_value, results);
    search_by_magnitude_exact_recursive(current_node->right, magnitude_value, results);
}
std::vector<EarthquakeRecord> KDTree::search_by_magnitude_exact(const std::string& magnitude_value) const {
    std::vector<EarthquakeRecord> results;
    search_by_magnitude_exact_recursive(root, magnitude_value, results);
    return results;
}

void KDTree::search_by_date_recursive(KDNode* current_node, const std::string& date_value, std::vector<EarthquakeRecord>& results) const {
    if (!current_node) return;
    if (current_node->record.date == date_value) {
        results.push_back(current_node->record);
    }
    search_by_date_recursive(current_node->left, date_value, results);
    search_by_date_recursive(current_node->right, date_value, results);
}
std::vector<EarthquakeRecord> KDTree::search_by_date(const std::string& date_value) const {
    std::vector<EarthquakeRecord> results;
    search_by_date_recursive(root, date_value, results);
    return results;
}

void KDTree::search_by_city_and_magnitude_recursive(KDNode* current_node, const std::string& city_name, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const {
    if (!current_node) return;
    if (current_node->record.city == city_name && current_node->record.magnitude == magnitude_value) {
        results.push_back(current_node->record);
    }
    search_by_city_and_magnitude_recursive(current_node->left, city_name, magnitude_value, results);
    search_by_city_and_magnitude_recursive(current_node->right, city_name, magnitude_value, results);
}
std::vector<EarthquakeRecord> KDTree::search_by_city_and_magnitude(const std::string& city_name, const std::string& magnitude_value) const {
    std::vector<EarthquakeRecord> results;
    search_by_city_and_magnitude_recursive(root, city_name, magnitude_value, results);
    return results;
}

// --- Funções de Listagem ---
void KDTree::get_all_records_recursive(KDNode* current_node, std::vector<EarthquakeRecord>& records_vec) const {
    if (current_node) {
        get_all_records_recursive(current_node->left, records_vec);
        records_vec.push_back(current_node->record);
        get_all_records_recursive(current_node->right, records_vec);
    }
}
std::vector<EarthquakeRecord> KDTree::get_all_records_vector() const {
    std::vector<EarthquakeRecord> all_records;
    get_all_records_recursive(root, all_records);
    return all_records;
}

void KDTree::list_all_records_recursive(KDNode* current_node, int& record_num) const {
    if (!current_node) return;
    list_all_records_recursive(current_node->left, record_num);
    std::cout << "Registro #" << record_num++ << ":" << std::endl;
    std::cout << "  "; 
    displayRecord(current_node->record); 
    std::cout << "  (Coords KD-Tree: Lat=" << std::fixed << std::setprecision(4) << current_node->point[0] 
              << ", Lon=" << std::fixed << std::setprecision(4) << current_node->point[1] << ")" << std::endl;
    if (node_count > 1 && record_num > 2) { // Evita linha extra no final ou para árvores pequenas
         std::cout << "  --------------------------------------------------------------------------------" << std::endl;
    }
    list_all_records_recursive(current_node->right, record_num);
}
void KDTree::list_all_records() const {
    if (is_empty()) {
        std::cout << "A KD-Tree esta vazia." << std::endl;
        return;
    }
    std::cout << "\n--- Todos os Registros de Terremotos na KD-Tree (" << node_count << ") ---" << std::endl;
    int record_num = 1;
    list_all_records_recursive(root, record_num);
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
}

void KDTree::list_by_min_magnitude_recursive(KDNode* current_node, double min_mag_val, int& total_found) const {
    if (!current_node) return;

    list_by_min_magnitude_recursive(current_node->left, min_mag_val, total_found);
    
    double current_rec_mag;
    // O terceiro argumento true suprime a mensagem de erro do helper, pois aqui só queremos filtrar
    if (KDTreeHelpers::try_string_to_double(current_node->record.magnitude, current_rec_mag, "", true)) {
        if (current_rec_mag >= min_mag_val) {
            total_found++;
            std::cout << "Registro Filtrado #" << total_found << ":" << std::endl;
            std::cout << "  "; displayRecord(current_node->record);
            if (total_found > 0) std::cout << "  --------------------------------------------------------------------------------" << std::endl;
        }
    }
    list_by_min_magnitude_recursive(current_node->right, min_mag_val, total_found);
}
void KDTree::list_by_min_magnitude(const std::string& min_mag_str) const {
    if (is_empty()) {
        std::cout << "A KD-Tree esta vazia. Nada para listar." << std::endl;
        return;
    }
    double min_mag_val;
    // O segundo argumento é o nome do campo para a mensagem de erro, não suprimimos aqui.
    if (!KDTreeHelpers::try_string_to_double(min_mag_str, min_mag_val, "Magnitude Minima", false)) {
        return;
    }
    std::cout << "\n--- Registros na KD-Tree com Magnitude >= " << std::fixed << std::setprecision(1) << min_mag_val << " ---" << std::endl;
    int total_found = 0;
    list_by_min_magnitude_recursive(root, min_mag_val, total_found);
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado com magnitude >= " << min_mag_str << "." << std::endl;
    }
     std::cout << "--------------------------------------------------------------------------------" << std::endl;
}

void KDTree::list_by_year_recursive(KDNode* current_node, const std::string& year_str, int& total_found) const {
    if (!current_node) return;
    list_by_year_recursive(current_node->left, year_str, total_found);
    if (current_node->record.date.rfind(year_str, 0) == 0) {
        total_found++;
        std::cout << "Registro Filtrado #" << total_found << ":" << std::endl;
        std::cout << "  "; displayRecord(current_node->record);
        if (total_found > 0) std::cout << "  --------------------------------------------------------------------------------" << std::endl;
    }
    list_by_year_recursive(current_node->right, year_str, total_found);
}
void KDTree::list_by_year(const std::string& year_str) const {
    if (is_empty()) {
        std::cout << "A KD-Tree esta vazia. Nada para listar." << std::endl;
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

    std::cout << "\n--- Registros na KD-Tree do Ano " << year_str << " ---" << std::endl;
    int total_found = 0;
    list_by_year_recursive(root, year_str, total_found);
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado para o ano " << year_str << "." << std::endl;
    }
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
}

// --- Helper para calcular distância ---
double KDTree::distance_squared(const double p1[K_DIMENSIONS], const double p2[K_DIMENSIONS]) const {
    double dist_sq = 0;
    for (int i = 0; i < K_DIMENSIONS; ++i) {
        dist_sq += std::pow(p1[i] - p2[i], 2);
    }
    return dist_sq;
}

// --- Funções de Busca Espacial ---
void KDTree::search_by_range_recursive(KDNode* current_node,
                                       double min_lat, double max_lat,
                                       double min_lon, double max_lon,
                                       unsigned depth,
                                       std::vector<EarthquakeRecord>& results) const {
    if (current_node == nullptr) {
        return;
    }

    double node_lat = current_node->point[0];
    double node_lon = current_node->point[1];

    if (node_lat >= min_lat && node_lat <= max_lat &&
        node_lon >= min_lon && node_lon <= max_lon) {
        results.push_back(current_node->record);
    }

    unsigned current_axis = depth % K_DIMENSIONS;
    double node_coord_on_axis = current_node->point[current_axis];
    double min_bound_on_axis = (current_axis == 0) ? min_lat : min_lon;
    double max_bound_on_axis = (current_axis == 0) ? max_lat : max_lon;

    if (min_bound_on_axis <= node_coord_on_axis) {
        search_by_range_recursive(current_node->left, min_lat, max_lat, min_lon, max_lon, depth + 1, results);
    }
    if (max_bound_on_axis >= node_coord_on_axis) {
        search_by_range_recursive(current_node->right, min_lat, max_lat, min_lon, max_lon, depth + 1, results);
    }
}

std::vector<EarthquakeRecord> KDTree::search_by_range(double min_lat, double max_lat,
                                                      double min_lon, double max_lon) const {
    std::vector<EarthquakeRecord> results;
    if (root == nullptr) {
        return results;
    }
    search_by_range_recursive(root, min_lat, max_lat, min_lon, max_lon, 0, results);
    return results;
}

void KDTree::nearest_neighbor_recursive(KDNode* current_node,
                                        const double target_point[K_DIMENSIONS],
                                        unsigned depth,
                                        KDNode*& best_match_node,
                                        double& min_distance_sq) const {
    if (current_node == nullptr) {
        return;
    }

    double d_sq = distance_squared(current_node->point, target_point);

    if (best_match_node == nullptr || d_sq < min_distance_sq) {
        min_distance_sq = d_sq;
        best_match_node = current_node;
    }

    unsigned current_axis = depth % K_DIMENSIONS;
    double diff_axis = target_point[current_axis] - current_node->point[current_axis];

    KDNode* first_child_to_visit = (diff_axis < 0) ? current_node->left : current_node->right;
    KDNode* second_child_to_visit = (diff_axis < 0) ? current_node->right : current_node->left;

    nearest_neighbor_recursive(first_child_to_visit, target_point, depth + 1, best_match_node, min_distance_sq);

    if (std::pow(diff_axis, 2) < min_distance_sq) {
        nearest_neighbor_recursive(second_child_to_visit, target_point, depth + 1, best_match_node, min_distance_sq);
    }
}

EarthquakeRecord* KDTree::search_nearest_neighbor(double target_lat, double target_lon) const {
    if (root == nullptr) {
        return nullptr;
    }
    double target_pt[K_DIMENSIONS] = {target_lat, target_lon};
    KDNode* best_node = nullptr;
    double min_dist_sq = std::numeric_limits<double>::max();

    nearest_neighbor_recursive(root, target_pt, 0, best_node, min_dist_sq);

    if (best_node) {
        return &(best_node->record);
    }
    return nullptr;
}

void KDTree::k_nearest_neighbors_recursive(KDNode* current_node,
                                           const double target_point[K_DIMENSIONS],
                                           unsigned k,
                                           unsigned depth,
                                           std::priority_queue<NeighborInfo>& k_best_neighbors) const {
    if (current_node == nullptr) {
        return;
    }

    double d_sq = distance_squared(current_node->point, target_point);

    if (k_best_neighbors.size() < k) {
        k_best_neighbors.push(NeighborInfo(current_node->record, d_sq));
    } else if (d_sq < k_best_neighbors.top().distance_sq) {
        k_best_neighbors.pop();
        k_best_neighbors.push(NeighborInfo(current_node->record, d_sq));
    }

    unsigned current_axis = depth % K_DIMENSIONS;
    double diff_axis = target_point[current_axis] - current_node->point[current_axis];

    KDNode* first_child_to_visit = (diff_axis < 0) ? current_node->left : current_node->right;
    KDNode* second_child_to_visit = (diff_axis < 0) ? current_node->right : current_node->left;

    k_nearest_neighbors_recursive(first_child_to_visit, target_point, k, depth + 1, k_best_neighbors);

    if (k_best_neighbors.size() < k || std::pow(diff_axis, 2) < k_best_neighbors.top().distance_sq) {
        k_nearest_neighbors_recursive(second_child_to_visit, target_point, k, depth + 1, k_best_neighbors);
    }
}

std::vector<EarthquakeRecord> KDTree::search_k_nearest_neighbors(double target_lat, double target_lon, unsigned k) const {
    std::vector<EarthquakeRecord> final_results; // Renomeado para clareza e para evitar confusão com o 'results' anterior no código original
    if (root == nullptr || k == 0) {
        return final_results;
    }

    double target_pt[K_DIMENSIONS] = {target_lat, target_lon};
    std::priority_queue<NeighborInfo> k_best_neighbors; // Assume que NeighborInfo::operator< está definido para max-heap

    k_nearest_neighbors_recursive(root, target_pt, k, 0, k_best_neighbors);

    // Extrai os resultados da priority_queue.
    // A priority_queue (max-heap) terá o mais distante dos k no topo.
    // Então, ao fazer pop, pegamos do mais distante para o mais próximo.
    // Para ter o mais próximo primeiro na lista final, precisamos inverter.
    final_results.reserve(k_best_neighbors.size());
    while(!k_best_neighbors.empty()) {
        final_results.push_back(k_best_neighbors.top().record);
        k_best_neighbors.pop();
    }
    std::reverse(final_results.begin(), final_results.end()); // Inverte para [mais_proximo, ..., k-esimo_mais_proximo]
    
    return final_results;
}