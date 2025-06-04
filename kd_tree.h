#ifndef KD_TREE_H
#define KD_TREE_H

#include "csv_reader.h" // Para EarthquakeRecord e displayRecord
#include <string>
#include <vector>
#include <iostream>   // Para std::cout, std::cerr em listagens/erros
#include <stdexcept>  // Para std::stod, std::invalid_argument, std::out_of_range
#include <algorithm>  // Para std::min, std::max, std::sort
#include <cmath>      // Para std::fabs, std::sqrt, std::pow (distância euclidiana)
#include <limits>     // Para std::numeric_limits
#include <queue>      // Para k-Nearest Neighbors (priority_queue)

// Dimensão da KD-Tree (Latitude, Longitude)
const int K_DIMENSIONS = 2;

struct KDNode {
    EarthquakeRecord record;
    double point[K_DIMENSIONS]; // [0] = latitude, [1] = longitude
    KDNode *left;
    KDNode *right;

    KDNode(const EarthquakeRecord& rec, double lat, double lon) : record(rec), left(nullptr), right(nullptr) {
        point[0] = lat;
        point[1] = lon;
    }
};

namespace KDTreeHelpers {
    // Helper para converter string para double com tratamento de erro
    inline bool try_string_to_double(const std::string& s, double& out_val, const std::string& field_name_for_error_msg = "", bool silent = false) {
        if (s.empty()) {
            if (!silent && !field_name_for_error_msg.empty()) {
                std::cerr << "AVISO (KDTree Conv): Campo '" << field_name_for_error_msg << "' vazio, nao pode ser convertido para double." << std::endl;
            }
            return false;
        }
        try {
            size_t processed_chars;
            out_val = std::stod(s, &processed_chars);
            for (size_t i = processed_chars; i < s.length(); ++i) {
                if (!std::isspace(static_cast<unsigned char>(s[i]))) {
                    if (!silent && !field_name_for_error_msg.empty()) {
                        std::cerr << "AVISO (KDTree Conv): Caracteres extras no campo '" << field_name_for_error_msg << "': '" << s << "'. Valor lido: " << out_val << std::endl;
                    }
                    // return false; // Para ser estrito com caracteres extras
                }
            }
            return true;
        } catch (const std::invalid_argument& ia) {
            if (!silent && !field_name_for_error_msg.empty()) {
                std::cerr << "AVISO (KDTree Conv): Argumento invalido para '" << field_name_for_error_msg << "': '" << s << "'. " << ia.what() << std::endl;
            }
        } catch (const std::out_of_range& oor) {
            if (!silent && !field_name_for_error_msg.empty()) {
                std::cerr << "AVISO (KDTree Conv): Valor fora do range para '" << field_name_for_error_msg << "': '" << s << "'. " << oor.what() << std::endl;
            }
        }
        return false;
    }

    // Helper para comparar dois EarthquakeRecords completos (para remoção precisa)
    inline bool are_records_identical(const EarthquakeRecord& r1, const EarthquakeRecord& r2) {
        // Compara todos os campos relevantes para identificar unicamente um registro
        return r1.date == r2.date &&
               r1.time == r2.time &&
               r1.city == r2.city &&
               r1.country == r2.country &&
               r1.magnitude == r2.magnitude && // Comparar como strings, pois são do dataset
               r1.depth == r2.depth &&
               r1.impact_score == r2.impact_score &&
               r1.latitude == r2.latitude &&   // Importante para garantir que é o mesmo ponto original
               r1.longitude == r2.longitude;
    }
} // namespace KDTreeHelpers

// Estrutura para auxiliar na busca por k-vizinhos mais próximos
struct NeighborInfo {
    EarthquakeRecord record;
    double distance_sq; // Distância ao quadrado (evita sqrt até o final)

    NeighborInfo(const EarthquakeRecord& rec, double dist_sq) : record(rec), distance_sq(dist_sq) {}

    // Para a priority_queue (max-heap, para manter os k menores)
    bool operator<(const NeighborInfo& other) const {
        return distance_sq < other.distance_sq;
    }
};

class KDTree {
private:
    KDNode* root;
    int node_count;

    // Funções auxiliares recursivas
    KDNode* insert_recursive(KDNode* current_node, const EarthquakeRecord& rec, double lat, double lon, unsigned depth);

    // Para a remoção, primeiro encontramos o registro exato, depois o removemos pela sua identidade e coordenadas
    bool find_exact_record_recursive(KDNode* current_node, const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country, EarthquakeRecord& found_record) const;
    KDNode* remove_by_exact_record_recursive(KDNode* current_node, const EarthquakeRecord& record_to_delete, double lat_del, double lon_del, unsigned depth);
    KDNode* find_min_recursive(KDNode* current_node, int target_axis, unsigned depth);

    void destroy_recursive(KDNode* current_node);

    // Helpers para buscas e listagens que varrem a árvore
    void get_all_records_recursive(KDNode* current_node, std::vector<EarthquakeRecord>& records_vec) const;
    void search_by_city_recursive(KDNode* current_node, const std::string& city_name, std::vector<EarthquakeRecord>& results) const;
    void search_by_magnitude_exact_recursive(KDNode* current_node, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const;
    void search_by_date_recursive(KDNode* current_node, const std::string& date_value, std::vector<EarthquakeRecord>& results) const;
    void search_by_city_and_magnitude_recursive(KDNode* current_node, const std::string& city_name, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const;

    void list_all_records_recursive(KDNode* current_node, int& record_num) const;
    void list_by_min_magnitude_recursive(KDNode* current_node, double min_mag_val, int& current_display_count, int& total_found) const;
    void list_by_year_recursive(KDNode* current_node, const std::string& year_str, int& current_display_count, int& total_found) const;

    // --- Funções auxiliares para buscas espaciais ---
    double distance_squared(const double p1[K_DIMENSIONS], const double p2[K_DIMENSIONS]) const;

    void search_by_range_recursive(KDNode* current_node,
                                   double min_lat, double max_lat,
                                   double min_lon, double max_lon,
                                   unsigned depth,
                                   std::vector<EarthquakeRecord>& results) const;

    void nearest_neighbor_recursive(KDNode* current_node,
                                    const double target_point[K_DIMENSIONS],
                                    unsigned depth,
                                    KDNode*& best_match_node,
                                    double& min_distance_sq) const;

    void k_nearest_neighbors_recursive(KDNode* current_node,
                                       const double target_point[K_DIMENSIONS],
                                       unsigned k,
                                       unsigned depth,
                                       std::priority_queue<NeighborInfo>& k_best_neighbors) const;

public:
    KDTree();
    ~KDTree();

    void insert_record(const EarthquakeRecord& record);
    bool remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country);

    std::vector<EarthquakeRecord> search_by_city(const std::string& city_name) const;
    std::vector<EarthquakeRecord> search_by_magnitude_exact(const std::string& magnitude_value) const;
    std::vector<EarthquakeRecord> search_by_date(const std::string& date_value) const;
    std::vector<EarthquakeRecord> search_by_city_and_magnitude(const std::string& city_name, const std::string& magnitude_value) const;

    void list_all_records() const;
    void list_by_min_magnitude(const std::string& min_mag_str) const;
    void list_by_year(const std::string& year_str) const;

    std::vector<EarthquakeRecord> get_all_records_vector() const;

    bool is_empty() const;
    int get_count() const;
    void clear_tree();

    // --- Funções de Busca Espacial ---
    std::vector<EarthquakeRecord> search_by_range(double min_lat, double max_lat,
                                                  double min_lon, double max_lon) const;

    EarthquakeRecord* search_nearest_neighbor(double target_lat, double target_lon) const;

    std::vector<EarthquakeRecord> search_k_nearest_neighbors(double target_lat, double target_lon, unsigned k) const;
};

#endif // KD_TREE_H