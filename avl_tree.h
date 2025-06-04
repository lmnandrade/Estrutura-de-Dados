#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "csv_reader.h" // For EarthquakeRecord
#include <string>
#include <vector>
#include <algorithm> // For std::max
#include <iostream>  // For list_... methods
#include <stdexcept> // For std::stod in list_by_min_magnitude

struct AVLNode {
    EarthquakeRecord record;
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const EarthquakeRecord& rec) : record(rec), left(nullptr), right(nullptr), height(1) {}
};

class AVLTree {
private:
    AVLNode* root;
    int node_count;

    std::string get_composite_key(const EarthquakeRecord& rec) const;
    std::string get_composite_key(const std::string& date, const std::string& time,
                                  const std::string& city, const std::string& country) const;

    int height(AVLNode* N);
    int get_balance(AVLNode* N);

    AVLNode* right_rotate(AVLNode* y);
    AVLNode* left_rotate(AVLNode* x);

    AVLNode* insert_node_recursive(AVLNode* node, const EarthquakeRecord& rec, const std::string& key);
    AVLNode* remove_node_recursive(AVLNode* current_node, const std::string& key_to_delete);
    AVLNode* find_min_value_node(AVLNode* node);

    AVLNode* search_node_recursive(AVLNode* node, const std::string& key) const;

    void get_all_records_recursive(AVLNode* node, std::vector<EarthquakeRecord>& result_vector) const; // Renamed from inorder_recursive
    void destroy_recursive(AVLNode* node);

    // Recursive helpers for search and list methods
    void search_by_city_recursive(AVLNode* node, const std::string& city_name, std::vector<EarthquakeRecord>& results) const;
    void search_by_magnitude_exact_recursive(AVLNode* node, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const;
    void search_by_date_recursive(AVLNode* node, const std::string& date_value, std::vector<EarthquakeRecord>& results) const;
    void search_by_city_and_magnitude_recursive(AVLNode* node, const std::string& city_name, const std::string& magnitude_value, std::vector<EarthquakeRecord>& results) const;

    void list_all_records_recursive(AVLNode* node, int& record_num) const;
    void list_by_min_magnitude_recursive(AVLNode* node, double min_mag_val, int& record_num, int& found_count) const;
    void list_by_year_recursive(AVLNode* node, const std::string& year_str, int& record_num, int& found_count) const;


public:
    AVLTree();
    ~AVLTree();

    // --- Métodos da Interface Comum ---
    void insert_record(const EarthquakeRecord& record);
    bool remove_record(const std::string& date, const std::string& time,
                       const std::string& city, const std::string& country);

    std::vector<EarthquakeRecord> search_by_city(const std::string& city_name) const;
    std::vector<EarthquakeRecord> search_by_magnitude_exact(const std::string& magnitude_value) const;
    std::vector<EarthquakeRecord> search_by_date(const std::string& date_value) const;
    std::vector<EarthquakeRecord> search_by_city_and_magnitude(const std::string& city_name, const std::string& magnitude_value) const;

    void list_all_records() const;
    void list_by_min_magnitude(const std::string& min_mag_str) const;
    void list_by_year(const std::string& year_str) const;

    std::vector<EarthquakeRecord> get_all_records() const; // Nome padronizado

    // --- Métodos Específicos da AVLTree (ou auxiliares públicos se necessário) ---
    const EarthquakeRecord* search_record_by_composite_key(const std::string& date, const std::string& time,
                                          const std::string& city, const std::string& country) const; // Original search
    bool is_empty() const;
    int get_count() const;
    void clear_tree();
};

#endif // AVL_TREE_H