import pandas as pd
from IPython.display import display

def load_data(file_path="earthquake_dataset.csv"):
    """Carrega dados de terremotos de um arquivo CSV."""
    try:
        df = pd.read_csv(file_path)
        return df
    except Exception as e:
        print(f"Erro ao carregar o arquivo CSV: {e}")
        return None

# Para uso direto (quando o arquivo é executado diretamente)
if __name__ == "__main__":
    c = "earthquake_dataset.csv"
    df = pd.read_csv(c)
    display(df.head())