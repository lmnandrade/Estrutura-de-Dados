#ifndef LISTA_DUPLA_LIGADA_H
#define LISTA_DUPLA_LIGADA_H

#include "csv_reader.h" // Para EarthquakeRecord
#include <string>
#include <vector>
#include <iostream>

struct Node {
    EarthquakeRecord data;
    Node* prev;
    Node* next;
    Node(const EarthquakeRecord& record) : data(record), prev(nullptr), next(nullptr) {}
};

class DoublyLinkedList {
private:
    Node* head;
    Node* tail;
    int count;
    void display_record_details(const EarthquakeRecord& record) const; // Usado por list_*

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    void insert_record(const EarthquakeRecord& record);
    bool remove_record(const std::string& date, const std::string& time_utc, const std::string& city, const std::string& country);
    
    std::vector<EarthquakeRecord> search_by_city(const std::string& city_name) const;
    std::vector<EarthquakeRecord> search_by_magnitude_exact(const std::string& magnitude_value) const;
    std::vector<EarthquakeRecord> search_by_date(const std::string& date_value) const;
    std::vector<EarthquakeRecord> search_by_city_and_magnitude(const std::string& city_name, const std::string& magnitude_value) const;
    // Para busca por nível de risco, a lógica será externa à classe, usando os resultados da classificação.

    void list_all_records() const;
    void list_by_min_magnitude(const std::string& min_mag_str) const;
    void list_by_year(const std::string& year_str) const;

    bool is_empty() const;
    int get_count() const;
    void clear_list();

    // Retorna todos os registros (pode ser útil para a busca por nível de risco)
    std::vector<EarthquakeRecord> get_all_records_vector() const;
};
#endif