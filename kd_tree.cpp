#include "kd_tree.h"
#include <iomanip> // Para formatação de saída em listagens

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

    unsigned current_axis = depth % K_DIMENSIONS;
    double new_point_coord = (current_axis == 0) ? lat : lon;
    double current_node_coord = current_node->point[current_axis];

    if (new_point_coord < current_node_coord) {
        current_node->left = insert_recursive(current_node->left, rec, lat, lon, depth + 1);
    } else {
        // Se as coordenadas forem iguais, podemos decidir ir para a direita (ou esquerda)
        // ou ter uma lógica mais complexa para lidar com pontos duplicados.
        // Para simplificar, vamos para a direita se for >=.
        // A verificação de duplicatas exatas (todo o EarthquakeRecord) pode ser feita antes de chamar insert.
        current_node->right = insert_recursive(current_node->right, rec, lat, lon, depth + 1);
    }
    return current_node;
}

void KDTree::insert_record(const EarthquakeRecord& record) {
    double lat, lon;
    if (!KDTreeHelpers::try_string_to_double(record.latitude, lat, "Latitude", true) ||
        !KDTreeHelpers::try_string_to_double(record.longitude, lon, "Longitude", true)) {
        std::cerr << "AVISO (KDTree Insercao): Nao foi possivel converter coordenadas para o registro: ";
        displayRecord(record); // Mostrar qual registro falhou
        std::cerr << "Registro nao inserido na KD-Tree." << std::endl;
        return;
    }
    // Opcional: Verificar se um registro *exatamente* igual já existe para evitar duplicatas
    // Isso exigiria uma busca por coordenadas e depois comparação do record.
    // Por simplicidade, a inserção atual permite múltiplos registros com mesmas coordenadas.
    root = insert_recursive(root, record, lat, lon, 0);
}

// --- Remoção ---

// Helper para encontrar o registro completo a ser deletado
bool KDTree::find_exact_record_recursive(KDNode* current_node,
                                         const std::string& date, const std::string& time_utc,
                                         const std::string& city, const std::string& country,
                                         EarthquakeRecord& found_record) const {
    if (!current_node) {
        return false;
    }
    // Verifica se o nó atual é o que queremos remover
    if (current_node->record.date == date &&
        current_node->record.time == time_utc &&
        current_node->record.city == city &&
        current_node->record.country == country) {
        found_record = current_node->record;
        return true;
    }
    // Continua buscando na árvore (não pode usar a estrutura da k-d tree para esta busca específica)
    // Uma busca em pre-ordem é suficiente para encontrar a primeira ocorrência.
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
        // Se estamos no eixo de interesse, o mínimo pode estar na subárvore esquerda
        if (current_node->left == nullptr) {
            return current_node;
        }
        return find_min_recursive(current_node->left, target_axis, depth + 1);
    } else {
        // Se não estamos no eixo de interesse, o mínimo pode estar em qualquer lugar
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

    // Chegamos ao nó que (potencialmente) queremos deletar?
    // Precisamos comparar tanto as coordenadas quanto o registro completo para ter certeza.
    // A comparação de float/double para igualdade exata pode ser problemática.
    // Usamos uma pequena tolerância EPSILON ou, mais robustamente, confiamos que
    // se as coordenadas são as mesmas E o record é o mesmo, então é o nó certo.
    const double EPSILON = 1e-9; // Pequena tolerância para comparação de double
    bool coords_match = (std::fabs(current_node->point[0] - lat_del) < EPSILON &&
                         std::fabs(current_node->point[1] - lon_del) < EPSILON);

    if (coords_match && KDTreeHelpers::are_records_identical(current_node->record, record_to_delete)) {
        // Nó encontrado, agora vamos removê-lo
        if (current_node->right != nullptr) {
            KDNode* min_node = find_min_recursive(current_node->right, current_axis, depth + 1);
            // Copiar dados do min_node para current_node
            current_node->record = min_node->record; // Cópia profunda do EarthquakeRecord
            current_node->point[0] = min_node->point[0];
            current_node->point[1] = min_node->point[1];
            // Remover recursivamente o min_node da subárvore direita
            current_node->right = remove_by_exact_record_recursive(current_node->right, min_node->record, min_node->point[0], min_node->point[1], depth + 1);
        } else if (current_node->left != nullptr) {
            // Se não há filho direito, o filho esquerdo (se existir) ou nullptr toma o lugar.
            KDNode* left_child = current_node->left;
            delete current_node;
            node_count--;
            return left_child; // O filho esquerdo se torna o nó nesta posição.
        } else { // Nó folha
            delete current_node;
            node_count--;
            return nullptr;
        }
    } else { // Navega para encontrar o nó
        if (point_to_delete_coord < current_node_coord) {
            current_node->left = remove_by_exact_record_recursive(current_node->left, record_to_delete, lat_del, lon_del, depth + 1);
        } else {
            // Se as coordenadas forem iguais, mas o registro não (are_records_identical falhou),
            // pode haver múltiplos registros no mesmo ponto. Precisamos checar ambos os lados
            // ou ter uma forma mais sofisticada de lidar com pontos duplicados.
            // Assumindo que se as coordenadas são iguais, e o registro não é o que buscamos,
            // ele deve estar na subárvore direita (devido à inserção).
            current_node->right = remove_by_exact_record_recursive(current_node->right, record_to_delete, lat_del, lon_del, depth + 1);
        }
    }
    return current_node;
}


bool KDTree::remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country) {
    if (is_empty()) return false;

    EarthquakeRecord record_to_delete;
    // Primeiro, encontrar o EarthquakeRecord completo que corresponde à chave
    if (!find_exact_record_recursive(root, date, time_utc, city, country, record_to_delete)) {
        // std::cout << "Registro para remocao nao encontrado na KD-Tree." << std::endl;
        return false; // Registro não encontrado
    }

    // Agora que temos o record_to_delete completo, converter suas coordenadas
    double lat_del, lon_del;
    if (!KDTreeHelpers::try_string_to_double(record_to_delete.latitude, lat_del, "Latitude (remocao)", true) ||
        !KDTreeHelpers::try_string_to_double(record_to_delete.longitude, lon_del, "Longitude (remocao)", true)) {
        std::cerr << "ERRO (KDTree Remocao): Nao foi possivel converter coordenadas do registro encontrado para remocao." << std::endl;
        return false; // Não deveria acontecer se foi inserido corretamente
    }
    
    int initial_count = node_count;
    root = remove_by_exact_record_recursive(root, record_to_delete, lat_del, lon_del, 0);
    
    return node_count < initial_count; // Retorna true se um nó foi removido
}


// --- Funções de Busca (varredura completa, pois a KD-Tree é para busca espacial) ---
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
    std::cout << "  "; // Indentação
    displayRecord(current_node->record); // Função global de csv_reader.h
    std::cout << "  (Coords KD-Tree: Lat=" << current_node->point[0] << ", Lon=" << current_node->point[1] << ")" << std::endl;
     if (record_num > 2 && node_count >1) std::cout << "  --------------------------------------------------------------------------------" << std::endl;
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
}

void KDTree::list_by_min_magnitude_recursive(KDNode* current_node, double min_mag_val, int& current_display_count, int& total_found) const {
    if (!current_node) return;

    list_by_min_magnitude_recursive(current_node->left, min_mag_val, current_display_count, total_found);
    
    double current_rec_mag;
    if (KDTreeHelpers::try_string_to_double(current_node->record.magnitude, current_rec_mag, "", true)) {
        if (current_rec_mag >= min_mag_val) {
            total_found++;
            std::cout << "Registro Filtrado #" << total_found << ":" << std::endl;
            std::cout << "  "; displayRecord(current_node->record);
            if (total_found > 1) std::cout << "  --------------------------------------------------------------------------------" << std::endl;
        }
    }
    list_by_min_magnitude_recursive(current_node->right, min_mag_val, current_display_count, total_found);
}
void KDTree::list_by_min_magnitude(const std::string& min_mag_str) const {
    if (is_empty()) {
        std::cout << "A KD-Tree esta vazia. Nada para listar." << std::endl;
        return;
    }
    double min_mag_val;
    if (!KDTreeHelpers::try_string_to_double(min_mag_str, min_mag_val, "Magnitude Minima")) {
        return; // Erro já foi impresso pelo helper
    }
    std::cout << "\n--- Registros na KD-Tree com Magnitude >= " << std::fixed << std::setprecision(1) << min_mag_val << " ---" << std::endl;
    int display_count = 0;
    int total_found = 0;
    list_by_min_magnitude_recursive(root, min_mag_val, display_count, total_found);
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado com magnitude >= " << min_mag_str << "." << std::endl;
    }
}

void KDTree::list_by_year_recursive(KDNode* current_node, const std::string& year_str, int& current_display_count, int& total_found) const {
    if (!current_node) return;
    list_by_year_recursive(current_node->left, year_str, current_display_count, total_found);
    if (current_node->record.date.rfind(year_str, 0) == 0) { // Verifica se a data começa com o ano
        total_found++;
        std::cout << "Registro Filtrado #" << total_found << ":" << std::endl;
        std::cout << "  "; displayRecord(current_node->record);
         if (total_found > 1) std::cout << "  --------------------------------------------------------------------------------" << std::endl;
    }
    list_by_year_recursive(current_node->right, year_str, current_display_count, total_found);
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
    try { std::stoi(year_str); } // Validar se é um número
    catch (const std::exception&) {
        std::cerr << "Erro: Ano invalido: " << year_str << std::endl;
        return;
    }

    std::cout << "\n--- Registros na KD-Tree do Ano " << year_str << " ---" << std::endl;
    int display_count = 0;
    int total_found = 0;
    list_by_year_recursive(root, year_str, display_count, total_found);
    if (total_found == 0) {
        std::cout << "Nenhum registro encontrado para o ano " << year_str << "." << std::endl;
    }
    
}
// --- Helper para calcular distância ---
double KDTree::distance_squared(const double p1[K_DIMENSIONS], const double p2[K_DIMENSIONS]) const {
    double dist_sq = 0;
    for (int i = 0; i < K_DIMENSIONS; ++i) {
        dist_sq += std::pow(p1[i] - p2[i], 2);
    }
    return dist_sq;
}

// ... (Implementações existentes) ...

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

    if (current_axis == 0) { // Latitude
        if (min_lat <= node_lat) { // A região de busca se estende ou inclui "abaixo" do plano do nó
            search_by_range_recursive(current_node->left, min_lat, max_lat, min_lon, max_lon, depth + 1, results);
        }
        if (max_lat >= node_lat) { // A região de busca se estende ou inclui "acima" do plano do nó
            search_by_range_recursive(current_node->right, min_lat, max_lat, min_lon, max_lon, depth + 1, results);
        }
    } else { // Longitude
        if (min_lon <= node_lon) { // A região de busca se estende ou inclui "à esquerda" do plano do nó
            search_by_range_recursive(current_node->left, min_lat, max_lat, min_lon, max_lon, depth + 1, results);
        }
        if (max_lon >= node_lon) { // A região de busca se estende ou inclui "à direita" do plano do nó
            search_by_range_recursive(current_node->right, min_lat, max_lat, min_lon, max_lon, depth + 1, results);
        }
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

    // Calcula a distância do nó atual ao ponto alvo
    double d_sq = distance_squared(current_node->point, target_point);

    // Se o nó atual é melhor que o melhor encontrado até agora
    if (best_match_node == nullptr || d_sq < min_distance_sq) {
        min_distance_sq = d_sq;
        best_match_node = current_node;
    }

    unsigned current_axis = depth % K_DIMENSIONS;
    double diff_axis = target_point[current_axis] - current_node->point[current_axis];

    KDNode* first_child_to_visit = (diff_axis < 0) ? current_node->left : current_node->right;
    KDNode* second_child_to_visit = (diff_axis < 0) ? current_node->right : current_node->left;

    // Visita a subárvore que contém o ponto alvo primeiro
    nearest_neighbor_recursive(first_child_to_visit, target_point, depth + 1, best_match_node, min_distance_sq);

    // Verifica se a outra subárvore precisa ser visitada (poda)
    // Se a distância ao quadrado do ponto alvo ao plano de divisão do nó atual
    // for menor que a menor distância encontrada até agora, então a outra subárvore PODE
    // conter um ponto mais próximo.
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
        // Retorna um ponteiro para o registro. Cuidado com o tempo de vida!
        // Se a árvore for modificada, este ponteiro pode se tornar inválido.
        // Uma cópia seria mais segura dependendo do uso.
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

    // Se a fila de prioridade ainda não tem k elementos, ou se o nó atual
    // é mais próximo que o k-ésimo mais distante na fila.
    if (k_best_neighbors.size() < k) {
        k_best_neighbors.push(NeighborInfo(current_node->record, d_sq));
    } else if (d_sq < k_best_neighbors.top().distance_sq) {
        k_best_neighbors.pop(); // Remove o mais distante dos k atuais
        k_best_neighbors.push(NeighborInfo(current_node->record, d_sq));
    }

    unsigned current_axis = depth % K_DIMENSIONS;
    double diff_axis = target_point[current_axis] - current_node->point[current_axis];

    KDNode* first_child_to_visit = (diff_axis < 0) ? current_node->left : current_node->right;
    KDNode* second_child_to_visit = (diff_axis < 0) ? current_node->right : current_node->left;

    k_nearest_neighbors_recursive(first_child_to_visit, target_point, k, depth + 1, k_best_neighbors);

    // Poda: só visita o segundo filho se a esfera ao redor do ponto alvo
    // (com raio igual à distância do k-ésimo vizinho mais distante encontrado até agora)
    // interceptar o plano de divisão do nó atual.
    // Ou, se a fila ainda não tem k vizinhos, sempre visita.
    if (k_best_neighbors.size() < k || std::pow(diff_axis, 2) < k_best_neighbors.top().distance_sq) {
        k_nearest_neighbors_recursive(second_child_to_visit, target_point, k, depth + 1, k_best_neighbors);
    }
}

std::vector<EarthquakeRecord> KDTree::search_k_nearest_neighbors(double target_lat, double target_lon, unsigned k) const {
    std::vector<EarthquakeRecord> results;
    if (root == nullptr || k == 0) {
        return results;
    }

    double target_pt[K_DIMENSIONS] = {target_lat, target_lon};
    // Usamos uma priority_queue como um max-heap para manter os k vizinhos com as menores distâncias.
    // O comparador em NeighborInfo fará com que o elemento com a maior distância_sq fique no topo.
    std::priority_queue<NeighborInfo> k_best_neighbors;

    k_nearest_neighbors_recursive(root, target_pt, k, 0, k_best_neighbors);

    // Extrai os resultados da priority_queue
    results.resize(k_best_neighbors.size());
    int i = results.size() - 1;
    while(!k_best_neighbors.empty()) {
        results[i--] = k_best_neighbors.top().record; // Adiciona na ordem correta de proximidade (mais próximo primeiro)
        k_best_neighbors.pop();
    }
    // Se você não se importar com a ordem no vetor de resultado, pode adicionar diretamente.
    // Para ter os mais próximos primeiro, precisa inverter ou inserir no início.
    // A linha acima preenche de trás para frente. Se quiser na ordem de mais próximo, precisa de um sort ou outra lógica.
    // A forma mais simples é empilhar e depois desempilhar:
    // std::vector<EarthquakeRecord> temp_results;
    // while(!k_best_neighbors.empty()) {
    //    temp_results.push_back(k_best_neighbors.top().record);
    //    k_best_neighbors.pop();
    // }
    // std::reverse(temp_results.begin(), temp_results.end());
    // return temp_results;

    // Para preencher na ordem correta sem reverse:
    // results.clear(); // Garante que está vazio
    // std::vector<NeighborInfo> temp_storage;
    // while(!k_best_neighbors.empty()) {
    //     temp_storage.push_back(k_best_neighbors.top());
    //     k_best_neighbors.pop();
    // }
    // // A priority queue é um max-heap, então o topo é o mais distante dos k.
    // // Para ter os mais próximos primeiro, iteramos de trás para frente no temp_storage.
    // for (auto it = temp_storage.rbegin(); it != temp_storage.rend(); ++it) {
    //     results.push_back(it->record);
    // }


    return results; 
                    std::vector<EarthquakeRecord> final_results;
                    final_results.reserve(k_best_neighbors.size());
                    while(!k_best_neighbors.empty()) {
                        final_results.push_back(k_best_neighbors.top().record);
                        k_best_neighbors.pop();
                    }
                    // A priority_queue (max-heap) terá o mais distante dos k no topo.
                    // Então, ao fazer pop, pegamos do mais distante para o mais próximo.
                    // Para ter o mais próximo primeiro, precisamos inverter.
                    std::reverse(final_results.begin(), final_results.end());
                    return final_results;
}
