/*
#include <iostream>     // Para std::cout e std::cerr
#include <list>         // Para std::list
#include <vector>       // Para std::vector (se lerCSV retorna vector)
#include <string>       // Para std::string
#include <algorithm>    // Para std::min
#include "csv_reader.h" // Inclui a interface da sua biblioteca CSV (assumindo struct Registro aqui)


int main() {
    std::string nomearqui = "C:/Users/letic/OneDrive/Documentos/Repositorio-github/Estrutura-de-Dados/earthquake_dataset.csv"; // Nome do arquivo CSV a ser lido

    // Chama a função da biblioteca para ler todos os dados
    // Assumimos que lerCSV retorna um std::vector<Registro>
    std::vector<Registro> data = lerCSV(nomearqui);

    // Verifica se a leitura foi bem-sucedida e se há dados
    if (data.empty()) {
        std::cerr << "Error: No data was read from the file or file is empty." << std::endl;
        // Considerar adicionar mais detalhes de erro em lerCSV
        return 1; // Retorna um código de erro indicando falha
    }

    // Cria uma lista duplamente ligada para armazenar os primeiros 5 registros
    std::list<Registro> listaRegistros;

    // Adiciona os primeiros 5 registros (ou menos, se o dataset for menor) para a lista
    // Usamos std::min para garantir que não acessamos índices fora do limite do vector 'data'
    size_t numeroRegistrosParaAdicionar = data.size();

    for (size_t i = 0; i < numeroRegistrosParaAdicionar; ++i) {
        // Adiciona uma cópia do registro do vector para o final da lista
        listaRegistros.push_back(data[i]);
    }

    // Exemplo de como acessar e imprimir os dados armazenados na lista
    std::cout << "Lista dos primeiros " << listaRegistros.size() << " registros:" << std::endl;

    // Itera sobre os elementos da lista usando um range-based for loop
    for (const auto& reg : listaRegistros) {
        std::cout << "Date: " << reg.date
                  << ", Time: " << reg.time
                  << ", City: " << reg.city
                  << ", Ctry: " << reg.ctry
                  << ", Lat: " << reg.lat
                  << ", Lon: " << reg.lon
                  << ", Mag: " << reg.mag
                  << ", Depth: " << reg.depth
                  << ", Imps: " << reg.imps << std::endl;
    }

    // Exemplo adicional: Acessando o primeiro e o último elemento (se existirem)
    if (!listaRegistros.empty()) {
        std::cout << "\nPrimeiro registro na lista (usando front()):" << std::endl;
        const Registro& primeiro = listaRegistros.front();
        std::cout << "Date: " << primeiro.date << ", City: " << primeiro.city << std::endl;

        std::cout << "Último registro na lista (usando back()):" << std::endl;
        const Registro& ultimo = listaRegistros.back();
        std::cout << "Date: " << ultimo.date << ", City: " << ultimo.city << std::endl;
    }


    return 0; // Indica que o programa executou com sucesso
}
    */

#include <iostream>     // Para std::cout e std::cerr
#include <string>       // Para std::string
#include <algorithm>    // Para std::min
// Função principal - única função main no código
int main() {
   int num;
   printf ("Type a number: ");
   scanf("%d", &num);
   printf("O número é %d", num);
}