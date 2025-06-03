#include "lista_functions.h"
#include <iostream>
#include <chrono>
#include <windows.h>

using namespace std;

// Executa benchmark de inserção, busca e remoção
BenchmarkResult runBenchmark(ListaDuplaLigada& lista, const std::vector<EarthquakeRecord>& records) {
    BenchmarkResult result = {0.0, 0.0, 0.0};
    
    if (records.empty()) {
        std::cout << "Não há registros para benchmark." << std::endl;
        return result;
    }
    
    // Limpa a lista antes do benchmark
    lista.clear();
    
    // Benchmark de inserção
    auto startInsert = std::chrono::high_resolution_clock::now();
    
    for (const auto& record : records) {
        lista.insertAtEnd(record);
    }
    
    auto endInsert = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> insertDuration = endInsert - startInsert;
    result.insertionTime = insertDuration.count();
    
    // Benchmark de busca (busca por cada cidade na lista)
    auto startSearch = std::chrono::high_resolution_clock::now();
    
    for (const auto& record : records) {
        lista.searchByCity(record.city);
    }
    
    auto endSearch = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> searchDuration = endSearch - startSearch;
    result.searchTime = searchDuration.count();
    
    // Benchmark de remoção (remove metade dos registros)
    auto startRemove = std::chrono::high_resolution_clock::now();
    
    int removeCount = records.size() / 2;
    for (int i = 0; i < removeCount && !lista.isEmpty(); i++) {
        Node* node = lista.getHead();
        lista.removeNode(node);
    }
    
    auto endRemove = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> removeDuration = endRemove - startRemove;
    result.removalTime = removeDuration.count();
    
    // Exibe os resultados
    std::cout << "=== BENCHMARK RESULTADOS ===" << std::endl;
    std::cout << "Tempo de inserção: " << result.insertionTime << " ms" << std::endl;
    std::cout << "Tempo de busca: " << result.searchTime << " ms" << std::endl;
    std::cout << "Tempo de remoção: " << result.removalTime << " ms" << std::endl;
    std::cout << "===========================" << std::endl;
    
    return result;
}

// Função para manipular a lista duplamente encadeada
void menuListaDupla(ListaDuplaLigada& lista) {
    int opcao;
    do {
        system("CLS");
        cout << "===== MENU LISTA DUPLAMENTE ENCADEADA =====\n\n";
        cout << "1- Monitorar terremotos\n";
        cout << "2- Alerta de terremoto\n";
        cout << "0- Voltar ao menu principal\n\n";
        cout << "Escolha uma opção: ";
        cin >> opcao;

        switch (opcao) {
            case 0:
                cout << "Voltando ao menu principal...\n";
                break;
            case 1: {
                // Menu para monitorar terremotos
                int subOpcao;
                do {
                    system("CLS");
                    cout << "===== MONITORAR TERREMOTOS =====\n\n";
                    cout << "1- Adicionar novo registro\n";
                    cout << "2- Remover registro\n";
                    cout << "3- Buscar registro\n";
                    cout << "4- Listar registros\n";
                    cout << "0- Voltar\n\n";
                    cout << "Escolha uma opção: ";
                    cin >> subOpcao;

                    switch (subOpcao) {
                        case 0:
                            cout << "Voltando...\n";
                            break;
                        case 1: {
                            // Adicionar novo registro
                            EarthquakeRecord newRecord;
                            cin.ignore(); // Limpa o buffer
                            
                            cout << "Data (DD/MM/YYYY): ";
                            getline(cin, newRecord.date);
                            
                            cout << "Hora (UTC): ";
                            getline(cin, newRecord.time);
                            
                            cout << "Cidade: ";
                            getline(cin, newRecord.city);
                            
                            cout << "País: ";
                            getline(cin, newRecord.country);
                            
                            cout << "Latitude: ";
                            getline(cin, newRecord.latitude);
                            
                            cout << "Longitude: ";
                            getline(cin, newRecord.longitude);
                            
                            cout << "Magnitude: ";
                            getline(cin, newRecord.magnitude);
                            
                            cout << "Profundidade (km): ";
                            getline(cin, newRecord.depth);
                            
                            cout << "Pontuação de Impacto: ";
                            getline(cin, newRecord.impactScore);
                            
                            lista.insertAtEnd(newRecord);
                            cout << "Registro adicionado com sucesso!\n";
                            system("PAUSE");
                            break;
                        }
                        case 2: {
                            // Remover registro
                            int removerOpcao;
                            cout << "Remover por:\n";
                            cout << "1- Cidade\n";
                            cout << "2- Magnitude\n";
                            cout << "3- Data\n";
                            cout << "Escolha: ";
                            cin >> removerOpcao;
                            
                            cin.ignore(); // Limpa o buffer
                            string valor;
                            
                            switch (removerOpcao) {
                                case 1: {
                                    cout << "Digite a cidade: ";
                                    getline(cin, valor);
                                    Node* node = lista.searchByCity(valor);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                        cout << "Deseja remover este registro? (1-Sim/0-Não): ";
                                        int confirma;
                                        cin >> confirma;
                                        if (confirma == 1) {
                                            lista.removeNode(node);
                                            cout << "Registro removido com sucesso!\n";
                                        }
                                    } else {
                                        cout << "Registro não encontrado.\n";
                                    }
                                    break;
                                }
                                case 2: {
                                    cout << "Digite a magnitude: ";
                                    getline(cin, valor);
                                    Node* node = lista.searchByMagnitude(valor);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                        cout << "Deseja remover este registro? (1-Sim/0-Não): ";
                                        int confirma;
                                        cin >> confirma;
                                        if (confirma == 1) {
                                            lista.removeNode(node);
                                            cout << "Registro removido com sucesso!\n";
                                        }
                                    } else {
                                        cout << "Registro não encontrado.\n";
                                    }
                                    break;
                                }
                                case 3: {
                                    cout << "Digite a data (DD/MM/YYYY): ";
                                    getline(cin, valor);
                                    Node* node = lista.searchByDate(valor);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                        cout << "Deseja remover este registro? (1-Sim/0-Não): ";
                                        int confirma;
                                        cin >> confirma;
                                        if (confirma == 1) {
                                            lista.removeNode(node);
                                            cout << "Registro removido com sucesso!\n";
                                        }
                                    } else {
                                        cout << "Registro não encontrado.\n";
                                    }
                                    break;
                                }
                                default:
                                    cout << "Opção inválida.\n";
                            }
                            system("PAUSE");
                            break;
                        }
                        case 3: {
                            // Buscar registro
                            int buscarOpcao;
                            cout << "Buscar por:\n";
                            cout << "1- Cidade\n";
                            cout << "2- Magnitude\n";
                            cout << "3- Data\n";
                            cout << "4- Nível de risco\n";
                            cout << "5- Cidade e magnitude\n";
                            cout << "Escolha: ";
                            cin >> buscarOpcao;
                            
                            cin.ignore(); // Limpa o buffer
                            
                            switch (buscarOpcao) {
                                case 1: {
                                    string cidade;
                                    cout << "Digite a cidade: ";
                                    getline(cin, cidade);
                                    Node* node = lista.searchByCity(cidade);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                    } else {
                                        cout << "Registro não encontrado.\n";
                                    }
                                    break;
                                }
                                case 2: {
                                    string magnitude;
                                    cout << "Digite a magnitude: ";
                                    getline(cin, magnitude);
                                    Node* node = lista.searchByMagnitude(magnitude);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                    } else {
                                        cout << "Registro não encontrado.\n";
                                    }
                                    break;
                                }
                                case 3: {
                                    string data;
                                    cout << "Digite a data (DD/MM/YYYY): ";
                                    getline(cin, data);
                                    Node* node = lista.searchByDate(data);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                    } else {
                                        cout << "Registro não encontrado.\n";
                                    }
                                    break;
                                }
                                case 4: {
                                    string nivelRisco;
                                    cout << "Digite o nível de risco (alto/médio/baixo): ";
                                    getline(cin, nivelRisco);
                                    Node* node = lista.searchByRiskLevel(nivelRisco);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                    } else {
                                        cout << "Registro não encontrado ou função não implementada.\n";
                                    }
                                    break;
                                }
                                case 5: {
                                    string cidade, magnitude;
                                    cout << "Digite a cidade: ";
                                    getline(cin, cidade);
                                    cout << "Digite a magnitude: ";
                                    getline(cin, magnitude);
                                    Node* node = lista.searchByCityAndMagnitude(cidade, magnitude);
                                    if (node) {
                                        cout << "Registro encontrado:\n";
                                        displayRecord(node->data);
                                    } else {
                                        cout << "Registro não encontrado.\n";
                                    }
                                    break;
                                }
                                default:
                                    cout << "Opção inválida.\n";
                            }
                            system("PAUSE");
                            break;
                        }                        case 4: {
                            // Listar registros
                            int listarOpcao;
                            cout << "Listar por:\n";
                            cout << "1- Magnitude mínima\n";
                            cout << "2- Ano\n";
                            cout << "3- Mostrar todos os registros\n";
                            cout << "Escolha: ";
                            cin >> listarOpcao;
                            
                            cin.ignore(); // Limpa o buffer
                            
                            switch (listarOpcao) {
                                case 1: {
                                    string minMagnitude;
                                    cout << "Digite a magnitude mínima: ";
                                    getline(cin, minMagnitude);
                                    lista.listByMinimumMagnitude(minMagnitude);
                                    break;
                                }
                                case 2: {
                                    string ano;
                                    cout << "Digite o ano: ";
                                    getline(cin, ano);
                                    lista.listByYear(ano);
                                    break;
                                }
                                case 3: {
                                    lista.displayAll();
                                    break;
                                }
                                default:
                                    cout << "Opção inválida.\n";
                            }
                            system("PAUSE");
                            break;
                        }
                        default:
                            cout << "Opção inválida.\n";
                            system("PAUSE");
                    }
                } while (subOpcao != 0);
                break;
            }
            case 2: {
                // Menu para alerta de terremoto
                system("CLS");
                cout << "===== ALERTA DE TERREMOTO =====\n\n";
                
                EarthquakeRecord newAlert;
                cin.ignore(); // Limpa o buffer
                
                cout << "Data (DD/MM/YYYY): ";
                getline(cin, newAlert.date);
                
                cout << "Hora (UTC): ";
                getline(cin, newAlert.time);
                
                cout << "Cidade: ";
                getline(cin, newAlert.city);
                
                cout << "País: ";
                getline(cin, newAlert.country);
                
                cout << "Distância do epicentro (km): ";
                string distanciaEpicentro;
                getline(cin, distanciaEpicentro);
                
                cout << "Email para alerta: ";
                string email;
                getline(cin, email);
                
                // Aqui você pode adicionar a lógica para calcular a magnitude estimada
                // e enviar o alerta com base nas regras definidas
                
                cout << "\nAlerta registrado com sucesso!\n";
                cout << "Um email seria enviado para " << email << " em caso de alerta necessário.\n";
                
                system("PAUSE");
                break;
            }
            default:
                cout << "\nOpção inválida\n\n";
                system("PAUSE");
        }
    } while (opcao != 0);
}