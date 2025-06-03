# main.py
import os
import platform
import time
import csv_reader
from lista_dupla_ligada import ListaDuplaEncadeada, converter_para_lista

def clear_screen():
    """Limpa a tela do console (compatível com Windows e Unix)."""
    if platform.system() == "Windows":
        os.system('cls')
    else:
        os.system('clear')

def pause():
    """Pausa o programa até que o usuário pressione Enter."""
    input("\nPressione Enter para continuar...")

def exibir_registro(registro, index=None):
    """Exibe os detalhes de um registro."""
    if index is not None:
        print(f"Registro #{index}:")
    
    print(f"Data: {registro.date}")
    print(f"Hora: {registro.time}")
    print(f"Cidade: {registro.city}")
    print(f"País: {registro.country}")
    print(f"Coordenadas: {registro.latitude}, {registro.longitude}")
    print(f"Magnitude: {registro.earthquake_magnitude}")
    print(f"Profundidade: {registro.depth} km")
    print(f"Impact Score: {registro.impact_score}\n")

def inserir_registro(lista_registros):
    """Insere um novo registro na lista e mede o tempo de execução."""
    from lista_dupla_ligada import Registro
    
    print("===== ADICIONAR NOVO REGISTRO =====\n")
    
    novo = Registro()
    novo.date = input("Digite a data (YYYY-MM-DD): ")
    novo.time = input("Digite o horário (UTC, HH:MM:SS): ")
    novo.city = input("Digite a cidade: ")
    novo.country = input("Digite o país: ")
    novo.latitude = input("Digite a latitude: ")
    novo.longitude = input("Digite a longitude: ")
    novo.earthquake_magnitude = input("Digite a magnitude do terremoto: ")
    novo.depth = input("Digite a profundidade (km): ")
    novo.impact_score = input("Digite o Impact Score: ")
    
    # Medir o tempo de inserção
    inicio = time.time()
    lista_registros.append(novo)
    fim = time.time()
    
    tempo_execucao = (fim - inicio) * 1000  # Converter para milissegundos
    
    print(f"\nRegistro adicionado com sucesso!")
    print(f"Tempo de execução: {tempo_execucao:.4f} ms")
    
    return lista_registros

def remover_registro(lista_registros):
    """Remove um registro da lista e mede o tempo de execução."""
    print("===== REMOVER REGISTRO =====\n")
    
    if lista_registros.is_empty():
        print("Lista vazia! Não há registros para remover.")
        return lista_registros
    
    try:
        indice = int(input(f"Digite o índice do registro a ser removido (0-{lista_registros.get_size() - 1}): "))
        
        # Medir o tempo de remoção
        inicio = time.time()
        resultado = lista_registros.remove(indice)
        fim = time.time()
        
        tempo_execucao = (fim - inicio) * 1000  # Converter para milissegundos
        
        if resultado:
            print("Registro removido com sucesso!")
        else:
            print("Índice inválido!")
        
        print(f"Tempo de execução: {tempo_execucao:.4f} ms")
    except ValueError:
        print("Por favor, digite um número válido.")
    
    return lista_registros

def buscar_por_cidade(lista_registros):
    """Busca registros por cidade e mede o tempo de execução."""
    print("===== BUSCAR REGISTRO POR CIDADE =====\n")
    
    cidade = input("Digite o nome da cidade: ")
    
    # Medir o tempo de busca
    inicio = time.time()
    resultados = lista_registros.buscar_por_cidade(cidade)
    fim = time.time()
    
    tempo_execucao = (fim - inicio) * 1000  # Converter para milissegundos
    
    if resultados:
        print(f"\nEncontrados {len(resultados)} registros para a cidade '{cidade}':")
        for i, reg in enumerate(resultados):
            exibir_registro(reg, i)
    else:
        print(f"Nenhum registro encontrado para a cidade: {cidade}")
    
    print(f"Tempo de execução: {tempo_execucao:.4f} ms")

def buscar_por_magnitude(lista_registros):
    """Busca registros por magnitude exata e mede o tempo de execução."""
    print("===== BUSCAR REGISTRO POR MAGNITUDE =====\n")
    
    magnitude = input("Digite a magnitude exata: ")
    
    # Medir o tempo de busca
    inicio = time.time()
    resultados = lista_registros.buscar_por_magnitude(magnitude)
    fim = time.time()
    
    tempo_execucao = (fim - inicio) * 1000  # Converter para milissegundos
    
    if resultados:
        print(f"\nEncontrados {len(resultados)} registros com magnitude {magnitude}:")
        for i, reg in enumerate(resultados):
            exibir_registro(reg, i)
    else:
        print(f"Nenhum registro encontrado com magnitude: {magnitude}")
    
    print(f"Tempo de execução: {tempo_execucao:.4f} ms")

def listar_por_magnitude_minima(lista_registros):
    """Lista registros com magnitude mínima e mede o tempo de execução."""
    print("===== LISTAR POR MAGNITUDE MÍNIMA =====\n")
    
    min_mag = input("Digite a magnitude mínima: ")
    
    # Medir o tempo de busca
    inicio = time.time()
    resultados = lista_registros.listar_por_magnitude_minima(min_mag)
    fim = time.time()
    
    tempo_execucao = (fim - inicio) * 1000  # Converter para milissegundos
    
    print(f"\nTerremotos com magnitude >= {min_mag}:\n")
    if resultados:
        print(f"Total de registros encontrados: {len(resultados)}")
        # Limitar a exibição para não sobrecarregar o console
        max_display = 10
        for i, reg in enumerate(resultados[:max_display]):
            exibir_registro(reg, i)
        
        if len(resultados) > max_display:
            print(f"... e mais {len(resultados) - max_display} registros.")
    else:
        print(f"Nenhum terremoto encontrado com magnitude maior ou igual a {min_mag}")
    
    print(f"Tempo de execução: {tempo_execucao:.4f} ms")

def listar_todos(lista_registros):
    """Lista todos os registros e mede o tempo de execução."""
    print("===== LISTAR TODOS OS REGISTROS =====\n")
    
    # Medir o tempo de listagem
    inicio = time.time()
    registros = lista_registros.get_all()
    fim = time.time()
    
    tempo_execucao = (fim - inicio) * 1000  # Converter para milissegundos
    
    if registros:
        print(f"Total de registros: {len(registros)}\n")
        
        # Limitar a exibição para não sobrecarregar o console
        max_display = 10
        for i, reg in enumerate(registros[:max_display]):
            exibir_registro(reg, i)
        
        if len(registros) > max_display:
            print(f"... e mais {len(registros) - max_display} registros.")
    else:
        print("Lista vazia! Não há registros para exibir.")
    
    print(f"Tempo de execução: {tempo_execucao:.4f} ms")

def menu_lista_dupla(lista_registros):
    """Menu para operações com a lista duplamente encadeada."""
    while True:
        clear_screen()
        print("===== LISTA DUPLAMENTE ENCADEADA =====\n")
        print(f"Total de registros: {lista_registros.get_size()}\n")
        print("1- Adicionar novo registro")
        print("2- Remover registro")
        print("3- Buscar registro por cidade")
        print("4- Buscar registro por magnitude")
        print("5- Listar por magnitude mínima")
        print("6- Listar todos os registros")
        print("0- Voltar ao menu principal")
        
        try:
            choice = int(input("\nDigite sua escolha: "))
            
            if choice == 0:
                return
            elif choice == 1:
                clear_screen()
                lista_registros = inserir_registro(lista_registros)
                pause()
            elif choice == 2:
                clear_screen()
                lista_registros = remover_registro(lista_registros)
                pause()
            elif choice == 3:
                clear_screen()
                buscar_por_cidade(lista_registros)
                pause()
            elif choice == 4:
                clear_screen()
                buscar_por_magnitude(lista_registros)
                pause()
            elif choice == 5:
                clear_screen()
                listar_por_magnitude_minima(lista_registros)
                pause()
            elif choice == 6:
                clear_screen()
                listar_todos(lista_registros)
                pause()
            else:
                print("Opção inválida!")
                pause()
        except ValueError:
            print("Por favor, digite um número válido.")
            pause()

def main():
    """Função principal do programa."""
    # Estruturas de dados
    lista_dupla = None
    
    while True:
        clear_screen()
        print("===== SISTEMA DE ANÁLISE DE TERREMOTOS =====\n")
        print("Escolha a estrutura de dados para trabalhar:")
        print("1- Lista Duplamente Encadeada")
        print("2- [Outra estrutura - a ser implementada]")
        print("3- [Outra estrutura - a ser implementada]")
        print("4- Carregar dataset")
        print("0- Sair")
        
        try:
            choice = int(input("\nDigite sua escolha: "))
            
            if choice == 0:
                print("Encerrando o programa...")
                break
            elif choice == 1:
                if lista_dupla is None:
                    print("Dataset não carregado. Por favor, carregue o dataset primeiro (opção 4).")
                    pause()
                else:
                    menu_lista_dupla(lista_dupla)
            elif choice == 2 or choice == 3:
                print("Estrutura ainda não implementada.")
                pause()
            elif choice == 4:
                clear_screen()
                print("===== CARREGAR DATASET =====\n")
                print("Carregando dataset...")
                
                try:
                    inicio = time.time()
                    df = csv_reader.load_data()
                    if df is not None:
                        lista_dupla = converter_para_lista(df)
                        fim = time.time()
                        
                        tempo_carregamento = (fim - inicio) * 1000  # Converter para milissegundos
                        
                        print(f"Dataset carregado com sucesso!")
                        print(f"Total de registros: {lista_dupla.get_size()}")
                        print(f"Tempo de carregamento: {tempo_carregamento:.4f} ms")
                    else:
                        print("Falha ao carregar o dataset.")
                except Exception as e:
                    print(f"Erro ao carregar o dataset: {e}")
                
                pause()
            else:
                print("Opção inválida!")
                pause()
        except ValueError:
            print("Por favor, digite um número válido.")
            pause()

if __name__ == "__main__":
    main()