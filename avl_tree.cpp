#include "avl_tree.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <queue>

// Construtor da Árvore AVL
ArvoreAVL::ArvoreAVL() {
    root = nullptr;
    size = 0;
    // Inicializar a chave de comparação padrão como cidade
    comparisonKey = "city";
}

// Destrutor da Árvore AVL
ArvoreAVL::~ArvoreAVL() {
    limparArvore(root);
}

// Função recursiva para limpar a árvore
void ArvoreAVL::limparArvore(NoAVL* no) {
    if (no != nullptr) {
        limparArvore(no->esquerda);
        limparArvore(no->direita);
        delete no;
    }
}

// Obter a altura de um nó
int ArvoreAVL::altura(NoAVL* no) {
    if (no == nullptr) {
        return 0;
    }
    return no->altura;
}

// Calcular o fator de balanceamento de um nó
int ArvoreAVL::fatorBalanceamento(NoAVL* no) {
    if (no == nullptr) {
        return 0;
    }
    return altura(no->esquerda) - altura(no->direita);
}

// Atualizar a altura de um nó
void ArvoreAVL::atualizarAltura(NoAVL* no) {
    if (no != nullptr) {
        no->altura = 1 + std::max(altura(no->esquerda), altura(no->direita));
    }
}

// Rotação simples à direita
NoAVL* ArvoreAVL::rotacaoDireita(NoAVL* y) {
    NoAVL* x = y->esquerda;
    NoAVL* T2 = x->direita;
    
    // Realizar rotação
    x->direita = y;
    y->esquerda = T2;
    
    // Atualizar alturas
    atualizarAltura(y);
    atualizarAltura(x);
    
    return x;
}

// Rotação simples à esquerda
NoAVL* ArvoreAVL::rotacaoEsquerda(NoAVL* x) {
    NoAVL* y = x->direita;
    NoAVL* T2 = y->esquerda;
    
    // Realizar rotação
    y->esquerda = x;
    x->direita = T2;
    
    // Atualizar alturas
    atualizarAltura(x);
    atualizarAltura(y);
    
    return y;
}

// Obter o valor da chave de comparação de um registro
std::string ArvoreAVL::getComparisonValue(const EarthquakeRecord& record) {
    if (comparisonKey == "city") {
        return record.city;
    } else if (comparisonKey == "country") {
        return record.country;
    } else if (comparisonKey == "date") {
        return record.date;
    } else if (comparisonKey == "magnitude") {
        return record.magnitude;
    } else {
        // Padrão: usar cidade
        return record.city;
    }
}

// Comparar dois registros com base na chave de comparação atual
int ArvoreAVL::compararRegistros(const EarthquakeRecord& a, const EarthquakeRecord& b) {
    std::string valorA = getComparisonValue(a);
    std::string valorB = getComparisonValue(b);
    
    // Comparação especial para magnitude (numérica)
    if (comparisonKey == "magnitude") {
        try {
            double magA = std::stod(valorA);
            double magB = std::stod(valorB);
            if (magA < magB) return -1;
            if (magA > magB) return 1;
            return 0;
        } catch (const std::exception& e) {
            // Em caso de erro, fazer comparação lexicográfica
        }
    }
    
    // Comparação lexicográfica para outros campos
    if (valorA < valorB) return -1;
    if (valorA > valorB) return 1;
    return 0;
}

// Inserir um registro na árvore AVL
NoAVL* ArvoreAVL::inserirNo(NoAVL* no, const EarthquakeRecord& record) {
    // Inserção normal em uma BST
    if (no == nullptr) {
        NoAVL* novoNo = new NoAVL;
        novoNo->record = record;
        novoNo->altura = 1;
        novoNo->esquerda = nullptr;
        novoNo->direita = nullptr;
        size++;
        return novoNo;
    }
    
    // Comparar registros para determinar onde inserir
    int comparacao = compararRegistros(record, no->record);
    
    if (comparacao < 0) {
        no->esquerda = inserirNo(no->esquerda, record);
    } else if (comparacao > 0) {
        no->direita = inserirNo(no->direita, record);
    } else {
        // Duplicatas: pode-se optar por não inserir ou adicionar em uma lista ligada no nó
        // Neste caso, vamos apenas retornar o nó sem modificação
        return no;
    }
    
    // Atualizar altura do nó atual
    atualizarAltura(no);
    
    // Verificar balanceamento e realizar rotações se necessário
    int balance = fatorBalanceamento(no);
    
    // Caso Esquerda-Esquerda
    if (balance > 1 && compararRegistros(record, no->esquerda->record) < 0) {
        return rotacaoDireita(no);
    }
    
    // Caso Direita-Direita
    if (balance < -1 && compararRegistros(record, no->direita->record) > 0) {
        return rotacaoEsquerda(no);
    }
    
    // Caso Esquerda-Direita
    if (balance > 1 && compararRegistros(record, no->esquerda->record) > 0) {
        no->esquerda = rotacaoEsquerda(no->esquerda);
        return rotacaoDireita(no);
    }
    
    // Caso Direita-Esquerda
    if (balance < -1 && compararRegistros(record, no->direita->record) < 0) {
        no->direita = rotacaoDireita(no->direita);
        return rotacaoEsquerda(no);
    }
    
    // Retornar o ponteiro do nó (sem alterações)
    return no;
}

// Encontrar o nó com o valor mínimo
NoAVL* ArvoreAVL::noValorMinimo(NoAVL* no) {
    NoAVL* atual = no;
    
    // Descer para a esquerda até encontrar o nó mais à esquerda
    while (atual->esquerda != nullptr) {
        atual = atual->esquerda;
    }
    
    return atual;
}

// Remover um nó da árvore AVL
NoAVL* ArvoreAVL::removerNo(NoAVL* no, const std::string& valor, const std::string& campo) {
    if (no == nullptr) {
        return no;
    }
    
    // Obter o valor do campo especificado do nó atual
    std::string valorNo;
    if (campo == "city") {
        valorNo = no->record.city;
    } else if (campo == "country") {
        valorNo = no->record.country;
    } else if (campo == "date") {
        valorNo = no->record.date;
    } else if (campo == "magnitude") {
        valorNo = no->record.magnitude;
    } else {
        valorNo = no->record.city; // Padrão
    }
    
    // Comparar valores para decidir onde continuar a busca
    if (valor < valorNo) {
        no->esquerda = removerNo(no->esquerda, valor, campo);
    } else if (valor > valorNo) {
        no->direita = removerNo(no->direita, valor, campo);
    } else {
        // Nó encontrado, realizar remoção
        
        // Caso 1: Nó com um ou nenhum filho
        if (no->esquerda == nullptr) {
            NoAVL* temp = no->direita;
            delete no;
            size--;
            return temp;
        } else if (no->direita == nullptr) {
            NoAVL* temp = no->esquerda;
            delete no;
            size--;
            return temp;
        }
        
        // Caso 2: Nó com dois filhos
        // Encontrar o sucessor in-order (menor valor na subárvore direita)
        NoAVL* temp = noValorMinimo(no->direita);
        
        // Copiar o conteúdo do sucessor para este nó
        no->record = temp->record;
        
        // Remover o sucessor
        no->direita = removerNo(no->direita, temp->record.city, "city");
    }
    
    // Se a árvore tinha apenas um nó, retornar
    if (no == nullptr) {
        return no;
    }
    
    // Atualizar altura do nó atual
    atualizarAltura(no);
    
    // Verificar balanceamento e realizar rotações se necessário
    int balance = fatorBalanceamento(no);
    
    // Caso Esquerda-Esquerda
    if (balance > 1 && fatorBalanceamento(no->esquerda) >= 0) {
        return rotacaoDireita(no);
    }
    
    // Caso Esquerda-Direita
    if (balance > 1 && fatorBalanceamento(no->esquerda) < 0) {
        no->esquerda = rotacaoEsquerda(no->esquerda);
        return rotacaoDireita(no);
    }
    
    // Caso Direita-Direita
    if (balance < -1 && fatorBalanceamento(no->direita) <= 0) {
        return rotacaoEsquerda(no);
    }
    
    // Caso Direita-Esquerda
    if (balance < -1 && fatorBalanceamento(no->direita) > 0) {
        no->direita = rotacaoDireita(no->direita);
        return rotacaoEsquerda(no);
    }
    
    return no;
}

// Buscar registros por valor em um campo específico
void ArvoreAVL::buscarPorValorRecursivo(NoAVL* no, const std::string& valor, const std::string& campo, std::vector<EarthquakeRecord>& resultados, int& comparacoes) {
    if (no == nullptr) {
        return;
    }
    
    comparacoes++;
    
    // Obter o valor do campo especificado do nó atual
    std::string valorNo;
    if (campo == "city") {
        valorNo = no->record.city;
    } else if (campo == "country") {
        valorNo = no->record.country;
    } else if (campo == "date") {
        valorNo = no->record.date;
    } else if (campo == "magnitude") {
        valorNo = no->record.magnitude;
    } else {
        valorNo = no->record.city; // Padrão
    }
    
    // Se for busca por magnitude, fazer comparação numérica
    if (campo == "magnitude") {
        try {
            double magValor = std::stod(valor);
            double magNo = std::stod(valorNo);
            
            if (magNo >= magValor) {
                resultados.push_back(no->record);
            }
            
            // Continuar a busca em ambas as subárvores para magnitude mínima
            buscarPorValorRecursivo(no->esquerda, valor, campo, resultados, comparacoes);
            buscarPorValorRecursivo(no->direita, valor, campo, resultados, comparacoes);
            return;
        } catch (const std::exception& e) {
            // Em caso de erro, fazer busca normal
        }
    }
    
    // Para outros campos, fazer busca exata
    if (valorNo == valor) {
        resultados.push_back(no->record);
    }
    
    // Decidir em quais subárvores continuar a busca
    if (valor < valorNo) {
        buscarPorValorRecursivo(no->esquerda, valor, campo, resultados, comparacoes);
    } else if (valor > valorNo) {
        buscarPorValorRecursivo(no->direita, valor, campo, resultados, comparacoes);
    } else {
        // Se encontrou uma correspondência exata, ainda precisa verificar duplicatas
        // em ambas as subárvores (se a árvore permitir duplicatas)
        buscarPorValorRecursivo(no->esquerda, valor, campo, resultados, comparacoes);
        buscarPorValorRecursivo(no->direita, valor, campo, resultados, comparacoes);
    }
}

// Buscar registros por cidade e magnitude
void ArvoreAVL::buscarPorCidadeEMagnitudeRecursivo(NoAVL* no, const std::string& cidade, const std::string& magnitudeMin, std::vector<EarthquakeRecord>& resultados, int& comparacoes) {
    if (no == nullptr) {
        return;
    }
    
    comparacoes++;
    
    // Verificar se o registro atual atende aos critérios
    if (no->record.city == cidade) {
        try {
            double magMin = std::stod(magnitudeMin);
            double magAtual = std::stod(no->record.magnitude);
            
            if (magAtual >= magMin) {
                resultados.push_back(no->record);
            }
        } catch (const std::exception& e) {
            // Ignorar se a conversão falhar
        }
    }
    
    // Continuar a busca em ambas as subárvores
    buscarPorCidadeEMagnitudeRecursivo(no->esquerda, cidade, magnitudeMin, resultados, comparacoes);
    buscarPorCidadeEMagnitudeRecursivo(no->direita, cidade, magnitudeMin, resultados, comparacoes);
}

// Percorrer a árvore em ordem (in-order)
void ArvoreAVL::percorrerEmOrdem(NoAVL* no, std::vector<EarthquakeRecord>& registros) {
    if (no != nullptr) {
        percorrerEmOrdem(no->esquerda, registros);
        registros.push_back(no->record);
        percorrerEmOrdem(no->direita, registros);
    }
}

// Percorrer a árvore em nível (level-order)
void ArvoreAVL::percorrerEmNivel(NoAVL* raiz, std::vector<EarthquakeRecord>& registros) {
    if (raiz == nullptr) {
        return;
    }
    
    std::queue<NoAVL*> fila;
    fila.push(raiz);
    
    while (!fila.empty()) {
        NoAVL* no = fila.front();
        fila.pop();
        
        registros.push_back(no->record);
        
        if (no->esquerda != nullptr) {
            fila.push(no->esquerda);
        }
        
        if (no->direita != nullptr) {
            fila.push(no->direita);
        }
    }
}

// Métodos públicos

// Definir a chave de comparação
void ArvoreAVL::definirChaveComparacao(const std::string& chave) {
    comparisonKey = chave;
}

// Inserir um registro na árvore
void ArvoreAVL::inserir(const EarthquakeRecord& record) {
    auto start = std::chrono::high_resolution_clock::now();
    
    root = inserirNo(root, record);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "Registro inserido com sucesso na Árvore AVL." << std::endl;
    std::cout << "Tempo de inserção: " << duration.count() << " ms" << std::endl;
}

// Remover um registro da árvore
bool ArvoreAVL::remover(const std::string& valor, const std::string& campo) {
    auto start = std::chrono::high_resolution_clock::now();
    
    int tamanhoAnterior = size;
    root = removerNo(root, valor, campo);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    bool removido = (tamanhoAnterior > size);
    
    if (removido) {
        std::cout << "Registro removido com sucesso da Árvore AVL." << std::endl;
    } else {
        std::cout << "Registro não encontrado na Árvore AVL." << std::endl;
    }
    
    std::cout << "Tempo de remoção: " << duration.count() << " ms" << std::endl;
    
    return removido;
}

// Buscar registros por cidade
std::vector<EarthquakeRecord> ArvoreAVL::buscarPorCidade(const std::string& cidade) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<EarthquakeRecord> resultados;
    int comparacoes = 0;
    
    // Se a árvore estiver indexada por cidade, usar busca eficiente
    if (comparisonKey == "city") {
        buscarPorValorRecursivo(root, cidade, "city", resultados, comparacoes);
    } else {
        // Caso contrário, percorrer toda a árvore
        std::vector<EarthquakeRecord> todosRegistros;
        percorrerEmOrdem(root, todosRegistros);
        
        for (const auto& record : todosRegistros) {
            comparacoes++;
            if (record.city == cidade) {
                resultados.push_back(record);
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "Busca por cidade na Árvore AVL concluída." << std::endl;
    std::cout << "Tempo de busca: " << duration.count() << " ms" << std::endl;
    std::cout << "Número de comparações: " << comparacoes << std::endl;
    std::cout << "Registros encontrados: " << resultados.size() << std::endl;
    
    return resultados;
}

// Buscar registros por magnitude mínima
std::vector<EarthquakeRecord> ArvoreAVL::buscarPorMagnitudeMinima(const std::string& magnitudeMin) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<EarthquakeRecord> resultados;
    int comparacoes = 0;
    
    // Se a árvore estiver indexada por magnitude, usar busca eficiente
    if (comparisonKey == "magnitude") {
        buscarPorValorRecursivo(root, magnitudeMin, "magnitude", resultados, comparacoes);
    } else {
        // Caso contrário, percorrer toda a árvore
        std::vector<EarthquakeRecord> todosRegistros;
        percorrerEmOrdem(root, todosRegistros);
        
        double magMin;
        try {
            magMin = std::stod(magnitudeMin);
        } catch (const std::exception& e) {
            std::cout << "Erro: Magnitude inválida." << std::endl;
            return resultados;
        }
        
        for (const auto& record : todosRegistros) {
            comparacoes++;
            try {
                double magAtual = std::stod(record.magnitude);
                if (magAtual >= magMin) {
                    resultados.push_back(record);
                }
            } catch (const std::exception& e) {
                // Ignorar registros com magnitude inválida
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "Busca por magnitude mínima na Árvore AVL concluída." << std::endl;
    std::cout << "Tempo de busca: " << duration.count() << " ms" << std::endl;
    std::cout << "Número de comparações: " << comparacoes << std::endl;
    std::cout << "Registros encontrados: " << resultados.size() << std::endl;
    
    return resultados;
}

// Buscar registros por data
std::vector<EarthquakeRecord> ArvoreAVL::buscarPorData(const std::string& data) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<EarthquakeRecord> resultados;
    int comparacoes = 0;
    
    // Se a árvore estiver indexada por data, usar busca eficiente
    if (comparisonKey == "date") {
        buscarPorValorRecursivo(root, data, "date", resultados, comparacoes);
    } else {
        // Caso contrário, percorrer toda a árvore
        std::vector<EarthquakeRecord> todosRegistros;
        percorrerEmOrdem(root, todosRegistros);
        
        for (const auto& record : todosRegistros) {
            comparacoes++;
            if (record.date == data) {
                resultados.push_back(record);
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "Busca por data na Árvore AVL concluída." << std::endl;
    std::cout << "Tempo de busca: " << duration.count() << " ms" << std::endl;
    std::cout << "Número de comparações: " << comparacoes << std::endl;
    std::cout << "Registros encontrados: " << resultados.size() << std::endl;
    
    return resultados;
}

// Buscar registros por cidade e magnitude
std::vector<EarthquakeRecord> ArvoreAVL::buscarPorCidadeEMagnitude(const std::string& cidade, const std::string& magnitudeMin) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<EarthquakeRecord> resultados;
    int comparacoes = 0;
    
    buscarPorCidadeEMagnitudeRecursivo(root, cidade, magnitudeMin, resultados, comparacoes);
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "Busca por cidade e magnitude na Árvore AVL concluída." << std::endl;
    std::cout << "Tempo de busca: " << duration.count() << " ms" << std::endl;
    std::cout << "Número de comparações: " << comparacoes << std::endl;
    std::cout << "Registros encontrados: " << resultados.size() << std::endl;
    
    return resultados;
}

// Listar todos os registros
void ArvoreAVL::listarTodos() {
    auto start = std::chrono::high_resolution_clock::now();
    
    if (root == nullptr) {
        std::cout << "Árvore vazia." << std::endl;
        return;
    }
    
    std::vector<EarthquakeRecord> registros;
    percorrerEmOrdem(root, registros);
    
    std::cout << "\n=== REGISTROS DE TERREMOTOS (ÁRVORE AVL) ===\n" << std::endl;
    std::cout << std::left << std::setw(12) << "Data" 
              << std::setw(10) << "Hora" 
              << std::setw(20) << "Cidade" 
              << std::setw(20) << "País" 
              << std::setw(10) << "Magnitude" 
              << std::setw(10) << "Profund." 
              << std::setw(15) << "Impact Score" << std::endl;
    
    std::cout << std::string(100, '-') << std::endl;
    
    int contador = 0;
    for (const auto& record : registros) {
        if (contador < 100) {  // Limitar a 100 registros para não sobrecarregar a saída
            std::cout << std::left << std::setw(12) << record.date 
                      << std::setw(10) << record.time 
                      << std::setw(20) << record.city 
                      << std::setw(20) << record.country 
                      << std::setw(10) << record.magnitude 
                      << std::setw(10) << record.depth 
                      << std::setw(15) << record.impact_score << std::endl;
            contador++;
        } else {
            break;
        }
    }
    
    if (registros.size() > 100) {
        std::cout << "\n... e mais " << (registros.size() - 100) << " registros (exibindo apenas os primeiros 100)." << std::endl;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "\nTotal de registros: " << registros.size() << std::endl;
    std::cout << "Tempo de listagem: " << duration.count() << " ms" << std::endl;
}

// Listar registros por ano
void ArvoreAVL::listarPorAno(const std::string& ano) {
    auto start = std::chrono::high_resolution_clock::now();
    
    if (root == nullptr) {
        std::cout << "Árvore vazia." << std::endl;
        return;
    }
    
    std::vector<EarthquakeRecord> todosRegistros;
    percorrerEmOrdem(root, todosRegistros);
    
    std::vector<EarthquakeRecord> registrosAno;
    
    for (const auto& record : todosRegistros) {
        // Extrair o ano da data (assumindo formato YYYY-MM-DD)
        std::string anoRegistro;
        if (record.date.length() >= 4) {
            anoRegistro = record.date.substr(0, 4);
        }
        
        if (anoRegistro == ano) {
            registrosAno.push_back(record);
        }
    }
    
    std::cout << "\n=== REGISTROS DE TERREMOTOS DO ANO " << ano << " (ÁRVORE AVL) ===\n" << std::endl;
    
    if (registrosAno.empty()) {
        std::cout << "Nenhum registro encontrado para o ano " << ano << "." << std::endl;
    } else {
        std::cout << std::left << std::setw(12) << "Data" 
                  << std::setw(10) << "Hora" 
                  << std::setw(20) << "Cidade" 
                  << std::setw(20) << "País" 
                  << std::setw(10) << "Magnitude" 
                  << std::setw(10) << "Profund." 
                  << std::setw(15) << "Impact Score" << std::endl;
        
        std::cout << std::string(100, '-') << std::endl;
        
        int contador = 0;
        for (const auto& record : registrosAno) {
            if (contador < 100) {  // Limitar a 100 registros para não sobrecarregar a saída
                std::cout << std::left << std::setw(12) << record.date 
                          << std::setw(10) << record.time 
                          << std::setw(20) << record.city 
                          << std::setw(20) << record.country 
                          << std::setw(10) << record.magnitude 
                          << std::setw(10) << record.depth 
                          << std::setw(15) << record.impact_score << std::endl;
                contador++;
            } else {
                break;
            }
        }
        
        if (registrosAno.size() > 100) {
            std::cout << "\n... e mais " << (registrosAno.size() - 100) << " registros (exibindo apenas os primeiros 100)." << std::endl;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "\nTotal de registros do ano " << ano << ": " << registrosAno.size() << std::endl;
    std::cout << "Tempo de listagem: " << duration.count() << " ms" << std::endl;
}
// Carregar registros do CSV para a árvore
void ArvoreAVL::carregarDoCSV(const std::vector<EarthquakeRecord>& records) {
    auto start = std::chrono::high_resolution_clock::now();
    
    for (const auto& record : records) {
        root = inserirNo(root, record);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    
    std::cout << "Registros carregados para a Árvore AVL." << std::endl;
    std::cout << "Total de registros carregados: " << size << std::endl;
    std::cout << "Tempo total de carregamento: " << duration.count() << " ms" << std::endl;
}

// Obter o tamanho da árvore
int ArvoreAVL::getTamanho() const {
    return size;
}

// Verificar se a árvore está vazia
bool ArvoreAVL::estaVazia() const {
    return root == nullptr;
}

// Imprimir a estrutura da árvore (para depuração)
void ArvoreAVL::imprimirArvore() {
    if (root == nullptr) {
        std::cout << "Árvore vazia." << std::endl;
        return;
    }
    
    std::cout << "\n=== ESTRUTURA DA ÁRVORE AVL ===\n" << std::endl;
    imprimirArvoreRecursivo(root, 0);
    std::cout << "\nAltura da árvore: " << altura(root) << std::endl;
    std::cout << "Número de nós: " << size << std::endl;
}
// Função auxiliar para imprimir a árvore recursivamente
void ArvoreAVL::imprimirArvoreRecursivo(NoAVL* no, int nivel) {
    if (no == nullptr) {
        return;
    }
    
    // Imprimir subárvore direita
    imprimirArvoreRecursivo(no->direita, nivel + 1);
    
    // Imprimir nó atual
    for (int i = 0; i < nivel; i++) {
        std::cout << "    ";
    }
    
    // Mostrar informações básicas do nó
    std::cout << no->record.city << " (" << no->record.magnitude << ") [FB: " 
              << fatorBalanceamento(no) << "]" << std::endl;
    
    // Imprimir subárvore esquerda
    imprimirArvoreRecursivo(no->esquerda, nivel + 1);
}

// Verificar se a árvore está balanceada
bool ArvoreAVL::estaBalanceada() {
    return verificarBalanceamentoRecursivo(root);
}

// Função auxiliar para verificar o balanceamento recursivamente
bool ArvoreAVL::verificarBalanceamentoRecursivo(NoAVL* no) {
    if (no == nullptr) {
        return true;
    }
    
    int fb = fatorBalanceamento(no);
    
    // Se o fator de balanceamento estiver fora do intervalo [-1, 1], a árvore não está balanceada
    if (fb < -1 || fb > 1) {
        return false;
    }
    
    // Verificar recursivamente as subárvores
    return verificarBalanceamentoRecursivo(no->esquerda) && verificarBalanceamentoRecursivo(no->direita);
}