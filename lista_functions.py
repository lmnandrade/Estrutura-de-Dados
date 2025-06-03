import time
from lista_dupla_ligada import Registro

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