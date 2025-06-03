import csv_reader

class Registro:
    """Classe para armazenar os dados de terremotos."""
    def __init__(self, date="", time="", city="", country="", latitude="", 
                 longitude="", earthquake_magnitude="", depth="", impact_score=""):
        self.date = date
        self.time = time
        self.city = city
        self.country = country
        self.latitude = latitude
        self.longitude = longitude
        self.earthquake_magnitude = earthquake_magnitude
        self.depth = depth
        self.impact_score = impact_score

class Node:
    """Nó da lista duplamente encadeada."""
    def __init__(self, data):
        self.data = data
        self.next = None
        self.prev = None

class ListaDuplaEncadeada:
    """Implementação de uma lista duplamente encadeada."""
    def __init__(self):
        self.head = None
        self.tail = None
        self.size = 0
    
    def append(self, data):
        """Adiciona um elemento ao final da lista."""
        new_node = Node(data)
        
        if self.head is None:
            self.head = new_node
            self.tail = new_node
        else:
            new_node.prev = self.tail
            self.tail.next = new_node
            self.tail = new_node
        
        self.size += 1
    
    def remove(self, index):
        """Remove um elemento pelo índice."""
        if index < 0 or index >= self.size:
            return False
        
        current = self.head
        for i in range(index):
            current = current.next
        
        if current.prev:
            current.prev.next = current.next
        else:
            self.head = current.next
        
        if current.next:
            current.next.prev = current.prev
        else:
            self.tail = current.prev
        
        self.size -= 1
        return True
    
    def get_size(self):
        """Retorna o tamanho da lista."""
        return self.size
    
    def is_empty(self):
        """Verifica se a lista está vazia."""
        return self.size == 0
    
    def buscar_por_cidade(self, cidade):
        """Busca registros por nome da cidade."""
        result = []
        current = self.head
        
        while current:
            if cidade.lower() in current.data.city.lower():  # Busca parcial case-insensitive
                result.append(current.data)
            current = current.next
        
        return result
    
    def buscar_por_magnitude(self, magnitude):
        """Busca registros por magnitude exata."""
        result = []
        current = self.head
        
        while current:
            try:
                if float(current.data.earthquake_magnitude) == float(magnitude):
                    result.append(current.data)
            except ValueError:
                pass  # Ignora registros com magnitude não numérica
            current = current.next
        
        return result
    
    def listar_por_magnitude_minima(self, min_magnitude):
        """Lista registros com magnitude maior ou igual à especificada."""
        result = []
        current = self.head
        
        while current:
            try:
                if float(current.data.earthquake_magnitude) >= float(min_magnitude):
                    result.append(current.data)
            except ValueError:
                pass  # Ignora registros com magnitude não numérica
            current = current.next
        
        return result
    
    def get_all(self):
        """Retorna todos os registros da lista."""
        result = []
        current = self.head
        
        while current:
            result.append(current.data)
            current = current.next
        
        return result

def carregar_dataset():
    """Carrega o dataset e converte para a lista duplamente encadeada."""
    import csv_reader
    df = csv_reader.load_data()
    if df is not None:
        return converter_para_lista(df)
    return None

def converter_para_lista(dataframe):
    """Converte um DataFrame pandas para uma ListaDuplaEncadeada."""
    lista = ListaDuplaEncadeada()
    
    for _, row in dataframe.iterrows():
        # Pula linhas totalmente vazias
        if row.isnull().all():
            continue
        
        # Verifica as colunas disponíveis no DataFrame
        date = str(row.get('Date', '')) if 'Date' in row else ''
        time = str(row.get('Time (UTC)', '')) if 'Time (UTC)' in row else ''
        city = str(row.get('City', '')) if 'City' in row else ''
        country = str(row.get('Country', '')) if 'Country' in row else ''
        latitude = str(row.get('Latitude', '')) if 'Latitude' in row else ''
        longitude = str(row.get('Longitude', '')) if 'Longitude' in row else ''
        earthquake_magnitude = str(row.get('Earthquake Magnitude', '')) if 'Earthquake Magnitude' in row else ''
        depth = str(row.get('Depth (km)', '')) if 'Depth (km)' in row else ''
        impact_score = str(row.get('Impact Score', '')) if 'Impact Score' in row else ''
        
        registro = Registro(
            date=date,
            time=time,
            city=city,
            country=country,
            latitude=latitude,
            longitude=longitude,
            earthquake_magnitude=earthquake_magnitude,
            depth=depth,
            impact_score=impact_score
        )
        
        # Só adiciona se tiver pelo menos uma informação relevante
        if any([date, city, country, earthquake_magnitude]):
            lista.append(registro)
    
    return lista

# Para teste direto deste módulo
if __name__ == "__main__":
    print("Carregando dataset para lista duplamente encadeada...")
    lista = carregar_dataset()
    if lista:
        print(f"Dataset carregado com sucesso! Total de registros: {lista.get_size()}")
        
        # Exibir alguns registros para teste
        registros = lista.get_all()
        for i, reg in enumerate(registros[:5]):  # Exibe os primeiros 5 registros
            print(f"\nRegistro #{i}:")
            print(f"Data: {reg.date}")
            print(f"Cidade: {reg.city}")
            print(f"País: {reg.country}")
            print(f"Magnitude: {reg.earthquake_magnitude}")
    else:
        print("Falha ao carregar o dataset.")