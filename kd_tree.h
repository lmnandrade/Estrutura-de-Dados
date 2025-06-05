#ifndef KD_TREE_H
#define KD_TREE_H

#include <string>
#include <vector>
#include <queue>    // Para std::priority_queue na declaração de k_nearest_neighbors_recursive
#include <iostream> // Para o namespace KDTreeHelpers se ele imprimir erros
#include "csv_reader.h" // Para EarthquakeRecord

// Definição da constante para as dimensões da KD-Tree
const int K_DIMENSIONS = 2;

// Forward declaration não é estritamente necessária para KDNode se definido antes de KDTree,
// mas não faz mal. Para NeighborInfo também.
struct KDNode;
struct NeighborInfo;
// EarthquakeRecord já vem de csv_reader.h

// Definição da estrutura do nó da KD-Tree
struct KDNode {
    EarthquakeRecord record;    // O registro completo do terremoto
    double point[K_DIMENSIONS]; // Coordenadas (ex: latitude, longitude)
    KDNode *left, *right;       // Filhos esquerdo e direito

    // Construtor do nó
    KDNode(const EarthquakeRecord& rec, double lat, double lon) : record(rec), left(nullptr), right(nullptr) {
        point[0] = lat;
        point[1] = lon;
    }
};

// Estrutura para auxiliar na busca dos K vizinhos mais próximos
struct NeighborInfo {
    EarthquakeRecord record;
    double distance_sq; // Distância ao quadrado do ponto de busca

    NeighborInfo(const EarthquakeRecord& r, double d_sq) : record(r), distance_sq(d_sq) {}

    // Sobrecarga do operador '<' para usar com std::priority_queue (max-heap)
    // Queremos que o elemento com a MAIOR distance_sq seja considerado "menor"
    // para que priority_queue.top() retorne o mais distante dos k atuais.
    bool operator<(const NeighborInfo& other) const {
        return distance_sq < other.distance_sq;
    }
};

// Declaração da classe KDTree
class KDTree {
private:
    KDNode* root;
    int node_count;

    // --- Métodos Privados Recursivos ---
    void destroy_recursive(KDNode* current_node);
    KDNode* insert_recursive(KDNode* current_node, const EarthquakeRecord& rec, double lat, double lon, unsigned depth);
    bool find_exact_record_recursive(KDNode* current_node,
                                     const std::string& date, const std::string& time_utc,
                                     const std::string& city, const std::string& country,
                                     EarthquakeRecord& found_record) const;
    KDNode* find_min_recursive(KDNode* current_node, int target_axis, unsigned depth);
    KDNode* remove_by_exact_record_recursive(KDNode* current_node,
                                           const EarthquakeRecord& record_to_delete,
                                           double lat_del, double lon_del, unsigned depth);
    void search_by_city_recursive(KDNode* current_node, const std::string& city_name, std::vector<EarthquakeRecord>& results) const;
    void search_by_magnitude_exact_recursive(KDNode* current_node, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const;
    void search_by_date_recursive(KDNode* current_node, const std::string& date_value, std::vector<EarthquakeRecord>& results) const;
    void search_by_city_and_magnitude_recursive(KDNode* current_node, const std::string& city_name, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const;
    void get_all_records_recursive(KDNode* current_node, std::vector<EarthquakeRecord>& records_vec) const;
    void list_all_records_recursive(KDNode* current_node, int& record_num) const;
    void list_by_min_magnitude_recursive(KDNode* current_node, double min_mag_val, int& total_found) const;
    void list_by_year_recursive(KDNode* current_node, const std::string& year_str, int& total_found) const;
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
    // --- Construtor, Destrutor e Gerenciamento Básico ---
    KDTree();
    ~KDTree();
    void clear_tree();
    bool is_empty() const;
    int get_count() const;

    // --- Operações de Modificação ---
    void insert_record(const EarthquakeRecord& record);
    bool remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country);
    
    // --- Operações de Busca por Atributo (varredura completa) ---
    std::vector<EarthquakeRecord> search_by_city(const std::string& city_name) const;
    std::vector<EarthquakeRecord> search_by_magnitude_exact(const std::string& magnitude_value) const;
    std::vector<EarthquakeRecord> search_by_date(const std::string& date_value) const;
    std::vector<EarthquakeRecord> search_by_city_and_magnitude(const std::string& city_name, const std::string& magnitude_value) const;
    
    // --- Operações de Listagem e Recuperação ---
    std::vector<EarthquakeRecord> get_all_records_vector() const;
    void list_all_records() const;
    void list_by_min_magnitude(const std::string& min_mag_str) const;
    void list_by_year(const std::string& year_str) const;

    // --- Operações de Busca Espacial ---
    std::vector<EarthquakeRecord> search_by_range(double min_lat, double max_lat, double min_lon, double max_lon) const;
    EarthquakeRecord* search_nearest_neighbor(double target_lat, double target_lon) const; // Retorna ponteiro, cuidado com o tempo de vida
    std::vector<EarthquakeRecord> search_k_nearest_neighbors(double target_lat, double target_lon, unsigned k) const;
};

// Namespace para funções auxiliares (declarações)
// As definições inline podem estar aqui ou as não-inline no kd_tree.cpp (ou um utils.cpp)
// Para este exemplo, as definições inline estão no kd_tree.cpp, então aqui são apenas declarações se não forem inline.
// Se as definições no kd_tree.cpp são 'inline', então elas devem estar no header.
// Pelo seu código, elas são inline no kd_tree.cpp, o que é um pouco incomum.
// Geralmente, helpers inline ficam completamente no header.
// Vamos assumir que você quer mantê-los no .cpp e aqui são apenas declarações se necessário.
// Se KDTreeHelpers são usados APENAS por kd_tree.cpp, não precisam ser declarados aqui.
// Se são usados por outros arquivos que incluem kd_tree.h, então sim.
// Pelo seu uso em kd_tree.cpp, parece que são helpers locais, então as definições no .cpp como você fez
// estão ok, DESDE QUE não sejam chamadas de fora do kd_tree.cpp sem uma declaração visível.
// Para simplificar e evitar problemas, se KDTreeHelpers é usado fora de kd_tree.cpp, coloque as definições inline aqui:

/*
namespace KDTreeHelpers {
    inline bool try_string_to_double(const std::string& s, double& out, const std::string& field_name_for_error = "", bool suppress_error_message = false) {
        // ... implementação como no .cpp ...
    }
    inline bool are_records_identical(const EarthquakeRecord& r1, const EarthquakeRecord& r2) {
        // ... implementação como no .cpp ...
    }
}
*/
// Se as funções KDTreeHelpers são usadas *apenas* dentro de kd_tree.cpp, então não há necessidade
// de declará-las no .h. O namespace com as definições inline no início do .cpp é suficiente para aquele arquivo.

#endif // KD_TREE_H