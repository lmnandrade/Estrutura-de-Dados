#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "csv_reader.h"
#include <vector>
#include <string>

// Estrutura para o nó da árvore AVL
struct NoAVL {
    EarthquakeRecord record;
    NoAVL* esquerda;
    NoAVL* direita;
    int altura;
};

// Classe para a árvore AVL
class ArvoreAVL {
private:
    NoAVL* root;          // Raiz da árvore
    int size;             // Número de nós na árvore
    std::string comparisonKey;  // Chave usada para comparação (city, country, date, magnitude)
    
    // Métodos auxiliares
    void limparArvore(NoAVL* no);
    int altura(NoAVL* no);
    int fatorBalanceamento(NoAVL* no);
    void atualizarAltura(NoAVL* no);
    NoAVL* rotacaoDireita(NoAVL* y);
    NoAVL* rotacaoEsquerda(NoAVL* x);
    std::string getComparisonValue(const EarthquakeRecord& record);
    int compararRegistros(const EarthquakeRecord& a, const EarthquakeRecord& b);
    NoAVL* inserirNo(NoAVL* no, const EarthquakeRecord& record);
    NoAVL* noValorMinimo(NoAVL* no);
    NoAVL* removerNo(NoAVL* no, const std::string& valor, const std::string& campo);
    void buscarPorValorRecursivo(NoAVL* no, const std::string& valor, const std::string& campo, std::vector<EarthquakeRecord>& resultados, int& comparacoes);
    void buscarPorCidadeEMagnitudeRecursivo(NoAVL* no, const std::string& cidade, const std::string& magnitudeMin, std::vector<EarthquakeRecord>& resultados, int& comparacoes);
    void percorrerEmOrdem(NoAVL* no, std::vector<EarthquakeRecord>& registros);
    void percorrerEmNivel(NoAVL* raiz, std::vector<EarthquakeRecord>& registros);
    void imprimirArvoreRecursivo(NoAVL* no, int nivel);
    bool verificarBalanceamentoRecursivo(NoAVL* no);
    
public:
    // Construtor e Destrutor
    ArvoreAVL();
    ~ArvoreAVL();
    // avl_tree.h
    void definirChaveComparacao(const std::string& chave);
    void carregarDoCSV(const std::vector<EarthquakeRecord>& registros);
    // Operações básicas
    void inserir(const EarthquakeRecord& record);
    bool remover(const std::string& valor, const std::string& campo);
    
    // Operações de busca
    std::vector<EarthquakeRecord> buscarPorCidade(const std::string& cidade);
    std::vector<EarthquakeRecord> buscarPorMagnitudeMinima(const std::string& magnitudeMin);
    std::vector<EarthquakeRecord> buscarPorData(const std::string& data);
    std::vector<EarthquakeRecord> buscarPorCidadeEMagnitude(const std::string& cidade, const std::string& magnitudeMin);
    
    // Operações de listagem
    void listarTodos();
    void listarPorAno(const std::string& ano);
    
    // Métodos auxiliares
    int getTamanho() const;
    bool estaVazia() const;
    void imprimirArvore();
    bool estaBalanceada();
};

#endif // AVL_TREE_H