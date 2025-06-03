#include "lista_dupla_ligada.h"
#include <iostream>
#include <cmath>

// Construtor
ListaDuplaLigada::ListaDuplaLigada() : head(nullptr), tail(nullptr), size(0) {}

// Destrutor
ListaDuplaLigada::~ListaDuplaLigada() {
    clear();
}

// Verifica se a lista está vazia
bool ListaDuplaLigada::isEmpty() const {
    return head == nullptr;
}

// Retorna o tamanho da lista
int ListaDuplaLigada::getSize() const {
    return size;
}

// Retorna o ponteiro para o início da lista
Node* ListaDuplaLigada::getHead() const {
    return head;
}

// Retorna o ponteiro para o final da lista
Node* ListaDuplaLigada::getTail() const {
    return tail;
}

// Insere um registro no final da lista
Node* ListaDuplaLigada::insertAtEnd(const EarthquakeRecord& record) {
    Node* newNode = new Node(record);
    
    if (isEmpty()) {
        head = newNode;
        tail = newNode;
    } else {
        newNode->prev = tail;
        tail->next = newNode;
        tail = newNode;
    }
    
    size++;
    return newNode;
}

// Insere um registro no início da lista
Node* ListaDuplaLigada::insertAtBeginning(const EarthquakeRecord& record) {
    Node* newNode = new Node(record);
    
    if (isEmpty()) {
        head = newNode;
        tail = newNode;
    } else {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
    }
    
    size++;
    return newNode;
}

// Função auxiliar para comparar strings numéricas
bool isGreaterThan(const std::string& a, const std::string& b) {
    try {
        double numA = std::stod(a);
        double numB = std::stod(b);
        return numA > numB;
    } catch (const std::exception& e) {
        // Se a conversão falhar, compare como strings
        return a > b;
    }
}

// Insere um registro de forma ordenada com base em um campo específico
Node* ListaDuplaLigada::insertSorted(const EarthquakeRecord& record, const std::string& sortField) {
    Node* newNode = new Node(record);
    
    // Se a lista estiver vazia, insere como primeiro elemento
    if (isEmpty()) {
        head = newNode;
        tail = newNode;
        size++;
        return newNode;
    }
    
    // Determina o valor a ser comparado com base no campo de ordenação
    std::string recordValue;
    if (sortField == "date") {
        recordValue = record.date;
    } else if (sortField == "magnitude") {
        recordValue = record.magnitude;
    } else if (sortField == "country") {
        recordValue = record.country;
    } else if (sortField == "city") {
        recordValue = record.city;
    } else {
        // Campo inválido, insere no final
        delete newNode;
        return insertAtEnd(record);
    }
    
    // Procura a posição correta para inserção
    Node* current = head;
    
    // Caso especial: inserir antes do primeiro nó
    std::string currentValue;
    if (sortField == "date") {
        currentValue = current->data.date;
    } else if (sortField == "magnitude") {
        currentValue = current->data.magnitude;
    } else if (sortField == "country") {
        currentValue = current->data.country;
    } else if (sortField == "city") {
        currentValue = current->data.city;
    }
    
    if (isGreaterThan(currentValue, recordValue)) {
        newNode->next = head;
        head->prev = newNode;
        head = newNode;
        size++;
        return newNode;
    }
    
    // Procura a posição correta no meio da lista
    while (current->next != nullptr) {
        if (sortField == "date") {
            currentValue = current->next->data.date;
        } else if (sortField == "magnitude") {
            currentValue = current->next->data.magnitude;
        } else if (sortField == "country") {
            currentValue = current->next->data.country;
        } else if (sortField == "city") {
            currentValue = current->next->data.city;
        }
        
        if (isGreaterThan(currentValue, recordValue)) {
            break;
        }
        
        current = current->next;
    }
    
    // Insere o novo nó após o nó atual
    newNode->next = current->next;
    newNode->prev = current;
    
    if (current->next != nullptr) {
        current->next->prev = newNode;
    } else {
        tail = newNode; // Se inserir no final, atualiza o tail
    }
    
    current->next = newNode;
    size++;
    
    return newNode;
}

// Remove um nó específico da lista
bool ListaDuplaLigada::removeNode(Node* node) {
    if (node == nullptr || isEmpty()) {
        return false;
    }
    
    // Caso especial: remover o único nó
    if (head == node && tail == node) {
        head = nullptr;
        tail = nullptr;
    }
    // Caso especial: remover o primeiro nó
    else if (head == node) {
        head = node->next;
        if (head != nullptr) {
            head->prev = nullptr;
        }
    }
    // Caso especial: remover o último nó
    else if (tail == node) {
        tail = node->prev;
        if (tail != nullptr) {
            tail->next = nullptr;
        }
    }
    // Caso geral: remover um nó do meio
    else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    
    delete node;
    size--;
    
    return true;
}

// Busca um registro por cidade
Node* ListaDuplaLigada::searchByCity(const std::string& city) const {
    if (isEmpty()) {
        return nullptr;
    }
    
    Node* current = head;
    
    while (current != nullptr) {
        if (current->data.city == city) {
            return current;
        }
        current = current->next;
    }
    
    return nullptr;
}

// Função auxiliar para extrair o ano de uma data
std::string extractYear(const std::string& date) {
    // Verifica se a data tem o tamanho mínimo necessário
    if (date.length() < 4) {
        return "";
    }
    
    // Procura por padrões comuns de data
    size_t firstSlash = date.find('/');
    size_t firstDash = date.find('-');
    
    // Se encontrou uma barra (formato DD/MM/YYYY ou MM/DD/YYYY)
    if (firstSlash != std::string::npos) {
        size_t secondSlash = date.find('/', firstSlash + 1);
        if (secondSlash != std::string::npos && secondSlash + 5 <= date.length()) {
            return date.substr(secondSlash + 1, 4);
        }
    }
    // Se encontrou um traço (formato YYYY-MM-DD)
    else if (firstDash != std::string::npos) {
        if (firstDash >= 4) {
            return date.substr(0, 4);
        }
    }
    // Se não encontrou separadores, tenta extrair os primeiros 4 caracteres
    else if (date.length() >= 4) {
        // Verifica se os primeiros 4 caracteres são dígitos
        bool allDigits = true;
        for (int i = 0; i < 4; i++) {
            if (!std::isdigit(date[i])) {
                allDigits = false;
                break;
            }
        }
        if (allDigits) {
            return date.substr(0, 4);
        }
    }
    
    return "";
}

// Busca um registro por magnitude
Node* ListaDuplaLigada::searchByMagnitude(const std::string& magnitude) const {
    if (isEmpty()) {
        return nullptr;
    }
    
    // Converte a magnitude de busca para double
    double searchMag;
    try {
        searchMag = std::stod(magnitude);
    } catch (const std::exception& e) {
        std::cout << "Erro ao converter magnitude de busca: " << magnitude << std::endl;
        return nullptr;
    }
    
    Node* current = head;
    
    while (current != nullptr) {
        // Converte a magnitude do registro atual para double
        double currentMag;
        try {
            currentMag = std::stod(current->data.magnitude);
            
            // Compara as magnitudes com uma pequena tolerância para evitar problemas de precisão
            if (std::abs(currentMag - searchMag) < 0.001) {
                return current;
            }
        } catch (const std::exception& e) {
            // Ignora registros com magnitude inválida
        }
        
        current = current->next;
    }
    
    return nullptr;
}

// Busca um registro por data
Node* ListaDuplaLigada::searchByDate(const std::string& date) const {
    if (isEmpty()) {
        return nullptr;
    }
    
    Node* current = head;
    
    while (current != nullptr) {
        if (current->data.date == date) {
            return current;
        }
        current = current->next;
    }
    
    return nullptr;
}

// Busca um registro por nível de risco
Node* ListaDuplaLigada::searchByRiskLevel(const std::string& riskLevel) const {
    // Esta é uma implementação simplificada
    // Na prática, você precisaria integrar com a função de cálculo de risco
    if (isEmpty()) {
        return nullptr;
    }
    
    // Aqui você pode adicionar a lógica para determinar o nível de risco
    // com base nos dados do registro
    
    return nullptr;
}

// Busca um registro por cidade e magnitude
Node* ListaDuplaLigada::searchByCityAndMagnitude(const std::string& city, const std::string& magnitude) const {
    if (isEmpty()) {
        return nullptr;
    }
    
    // Converte a magnitude de busca para double
    double searchMag;
    try {
        searchMag = std::stod(magnitude);
    } catch (const std::exception& e) {
        std::cout << "Erro ao converter magnitude de busca: " << magnitude << std::endl;
        return nullptr;
    }
    
    Node* current = head;
    
    while (current != nullptr) {
        if (current->data.city == city) {
            // Converte a magnitude do registro atual para double
            try {
                double currentMag = std::stod(current->data.magnitude);
                
                // Compara as magnitudes com uma pequena tolerância
                if (std::abs(currentMag - searchMag) < 0.001) {
                    return current;
                }
            } catch (const std::exception& e) {
                // Ignora registros com magnitude inválida
            }
        }
        current = current->next;
    }
    
    return nullptr;
}

// Lista registros com magnitude maior ou igual a um valor mínimo
void ListaDuplaLigada::listByMinimumMagnitude(const std::string& minMagnitude) const {
    if (isEmpty()) {
        std::cout << "A lista está vazia." << std::endl;
        return;
    }
    
    // Converte a magnitude mínima para double
    double minMag;
    try {
        minMag = std::stod(minMagnitude);
    } catch (const std::exception& e) {
        std::cout << "Magnitude inválida: " << minMagnitude << std::endl;
        return;
    }
    
    Node* current = head;
    int count = 0;
    
    std::cout << "Terremotos com magnitude >= " << minMagnitude << ":" << std::endl;
    
    while (current != nullptr) {
        // Converte a magnitude do registro atual para double
        try {
            double currentMag = std::stod(current->data.magnitude);
            
            if (currentMag >= minMag) {
                count++;
                std::cout << "Registro #" << count << std::endl;
                displayRecord(current->data);
            }
        } catch (const std::exception& e) {
            // Ignora registros com magnitude inválida
        }
        
        current = current->next;
    }
    
    if (count == 0) {
        std::cout << "Nenhum terremoto encontrado com magnitude >= " << minMagnitude << std::endl;
    } else {
        std::cout << "Total de registros encontrados: " << count << std::endl;
    }
}

// Lista registros por ano
void ListaDuplaLigada::listByYear(const std::string& year) const {
    if (isEmpty()) {
        std::cout << "A lista está vazia." << std::endl;
        return;
    }
    
    Node* current = head;
    int count = 0;
    
    std::cout << "Terremotos ocorridos no ano " << year << ":" << std::endl;
    
    while (current != nullptr) {
        std::string recordYear = extractYear(current->data.date);
        
        if (recordYear == year) {
            count++;
            std::cout << "Registro #" << count << std::endl;
            displayRecord(current->data);
        }
        
        current = current->next;
    }
    
    if (count == 0) {
        std::cout << "Nenhum terremoto encontrado no ano " << year << std::endl;
    } else {
        std::cout << "Total de registros encontrados: " << count << std::endl;
    }
}

// Limpa a lista (remove todos os nós)
void ListaDuplaLigada::clear() {
    Node* current = head;
    Node* next;
    
    while (current != nullptr) {
        next = current->next;
        delete current;
        current = next;
    }
    
    head = nullptr;
    tail = nullptr;
    size = 0;
}

// Exibe todos os registros da lista
void ListaDuplaLigada::displayAll() const {
    if (isEmpty()) {
        std::cout << "A lista está vazia." << std::endl;
        return;
    }
    
    std::cout << "Total de registros na lista: " << size << std::endl;
    
    Node* current = head;
    int count = 1;
    
    while (current != nullptr) {
        std::cout << "Registro #" << count << std::endl;
        displayRecord(current->data);
        current = current->next;
        count++;
    }
}