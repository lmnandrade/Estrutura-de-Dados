#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include "csv_reader.h"
#include "lista_dupla_ligada.h"
#include "lista_functions.h"

using namespace std;

int main() {
    string nomearquivo = "C:/Users/letic/OneDrive/Documentos/Repositorio-github/Estrutura-de-Dados/earthquake_dataset.csv";

    // Lê os dados do CSV
    vector<EarthquakeRecord> records = readEarthquakeCSV(nomearquivo);

    // Verifica se a leitura foi bem-sucedida
    if (records.empty()) {
        cerr << "Erro: Nenhum dado foi lido do arquivo ou o arquivo está vazio." << endl;
        return 1;
    }

    // Cria uma lista duplamente encadeada e carrega os dados
    ListaDuplaLigada lista;
    for (const auto& record : records) {
        lista.insertAtEnd(record);
    }
    
    cout << "Dados carregados com sucesso! " << lista.getSize() 
         << " registros encontrados." << endl;
    system("PAUSE");

    unsigned short int escolha;
    do {
        system("CLS");
        cout << "===== MENU PRINCIPAL =====\n\n";
        cout << "1- Lista Duplamente Encadeada\n";
        cout << "2- Executar Benchmark\n";
        cout << "0- Sair\n\n";
        cout << "Escolha sua estrutura: ";
        cin >> escolha;

        switch(escolha) {
            case 0:
                cout << "Saindo do programa...\n";
                break;
            case 1:
                system("CLS"); 
                menuListaDupla(lista);
                break;
            case 2: {
                system("CLS");
                cout << "Executando benchmark...\n";
                BenchmarkResult result = runBenchmark(lista, records);
                system("PAUSE");
                break;
            }
            default:
                cout << "\nEscolha inválida\n\n";
                system("PAUSE");
                break;
        }
    } while (escolha != 0);
    
    return 0;
}