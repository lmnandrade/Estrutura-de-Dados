#ifndef LISTA_DUPLA_LIGADA_H
#define LISTA_DUPLA_LIGADA_H

#include "csv_reader.h"
#include <string>

// Estrutura do nó da lista duplamente encadeada
struct Node {
    EarthquakeRecord data;
    Node* next;
    Node* prev;
    
    // Construtor
    Node(const EarthquakeRecord& record) : data(record), next(nullptr), prev(nullptr) {}
};

// Classe da lista duplamente encadeada
class ListaDuplaLigada {
private:
    Node* head;
    Node* tail;
    int size;

public:
    // Construtor e destrutor
    ListaDuplaLigada();
    ~ListaDuplaLigada();
    
    // Métodos básicos
    bool isEmpty() const;
    int getSize() const;
    Node* getHead() const;
    Node* getTail() const;
    
    // Métodos para manipulação da lista
    Node* insertAtEnd(const EarthquakeRecord& record);
    Node* insertAtBeginning(const EarthquakeRecord& record);
    Node* insertSorted(const EarthquakeRecord& record, const std::string& sortField);
    bool removeNode(Node* node);
    
    // Métodos de busca
    Node* searchByCity(const std::string& city) const;
    Node* searchByMagnitude(const std::string& magnitude) const;
    Node* searchByDate(const std::string& date) const;
    Node* searchByRiskLevel(const std::string& riskLevel) const;
    Node* searchByCityAndMagnitude(const std::string& city, const std::string& magnitude) const;
    
    // Métodos de listagem
    void listByMinimumMagnitude(const std::string& minMagnitude) const;
    void listByYear(const std::string& year) const;
    
    // Método para limpar a lista
    void clear();
    
    // Método para exibir todos os registros da lista
    void displayAll() const;
};

#endif // LISTA_DUPLA_LIGADA_H