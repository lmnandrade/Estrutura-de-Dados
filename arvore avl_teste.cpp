#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <limits> // Para std::numeric_limits

// Estrutura para armazenar dados de terremotos
struct Terremoto {
    std::string data;
    std::string hora;
    std::string cidade;
    std::string pais;
    double latitude;
    double longitude;
    double magnitude;
    double profundidade;
    double impacto;

    // Construtor padrão
    Terremoto() : latitude(0), longitude(0), magnitude(0), profundidade(0), impacto(0) {}

    // Construtor com parâmetros
    Terremoto(const std::string& data, const std::string& hora, const std::string& cidade,
              const std::string& pais, double latitude, double longitude, 
              double magnitude, double profundidade, double impacto)
        : data(data), hora(hora), cidade(cidade), pais(pais), latitude(latitude),
          longitude(longitude), magnitude(magnitude), profundidade(profundidade), impacto(impacto) {}

    // Método para exibir informações do terremoto
    void exibir() const {
        std::cout << "Data: " << data << " | Hora: " << hora << " | Local: " << cidade << ", " << pais << std::endl;
        std::cout << "Coordenadas: (" << latitude << ", " << longitude << ") | Magnitude: " << magnitude << std::endl;
        std::cout << "Profundidade: " << profundidade << " km | Impacto: " << impacto << std::endl;
        std::cout << "Classificação de risco: " << classificarRisco() << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
    }

    // Método para classificar o risco do terremoto
    std::string classificarRisco() const {
        // Classificação baseada na magnitude e impacto
        if (magnitude >= 7.0 || impacto >= 8.0) {
            return "Alto";
        } else if (magnitude >= 5.0 || impacto >= 5.0) {
            return "Médio";
        } else {
            return "Baixo";
        }
    }
};

// Nó da árvore AVL
struct NoAVL {
    Terremoto terremoto;
    NoAVL* esquerda;
    NoAVL* direita;
    int altura;

    // Construtor
    NoAVL(const Terremoto& t) : terremoto(t), esquerda(nullptr), direita(nullptr), altura(1) {}
};

// Classe da árvore AVL
class ArvoreAVL {
private:
    NoAVL* raiz;

    // Função auxiliar para obter a altura de um nó
    int getAltura(NoAVL* no) {
        if (no == nullptr) return 0;
        return no->altura;
    }

    // Função para calcular o fator de balanceamento
    int getFatorBalanceamento(NoAVL* no) {
        if (no == nullptr) return 0;
        return getAltura(no->esquerda) - getAltura(no->direita);
    }

    // Função para atualizar a altura de um nó
    void atualizarAltura(NoAVL* no) {
        if (no == nullptr) return;
        no->altura = 1 + std::max(getAltura(no->esquerda), getAltura(no->direita));
    }

    // Rotação à direita
    NoAVL* rotacaoDireita(NoAVL* y) {
        NoAVL* x = y->esquerda;
        NoAVL* T2 = x->direita;

        // Realiza a rotação
        x->direita = y;
        y->esquerda = T2;

        // Atualiza alturas
        atualizarAltura(y);
        atualizarAltura(x);

        return x;
    }

    // Rotação à esquerda
    NoAVL* rotacaoEsquerda(NoAVL* x) {
        NoAVL* y = x->direita;
        NoAVL* T2 = y->esquerda;

        // Realiza a rotação
        y->esquerda = x;
        x->direita = T2;

        // Atualiza alturas
        atualizarAltura(x);
        atualizarAltura(y);

        return y;
    }

    // Função para balancear a árvore após inserção/remoção
    NoAVL* balancear(NoAVL* no, const Terremoto& t) {
        if (no == nullptr) return nullptr;

        // Atualiza a altura do nó atual
        atualizarAltura(no);

        // Obtém o fator de balanceamento
        int fator = getFatorBalanceamento(no);

        // Casos de desbalanceamento
        
        // Caso Esquerda-Esquerda
        if (fator > 1 && compararTerremotos(t, no->esquerda->terremoto) < 0)
            return rotacaoDireita(no);

        // Caso Direita-Direita
        if (fator < -1 && compararTerremotos(t, no->direita->terremoto) > 0)
            return rotacaoEsquerda(no);

        // Caso Esquerda-Direita
        if (fator > 1 && compararTerremotos(t, no->esquerda->terremoto) > 0) {
            no->esquerda = rotacaoEsquerda(no->esquerda);
            return rotacaoDireita(no);
        }

        // Caso Direita-Esquerda
        if (fator < -1 && compararTerremotos(t, no->direita->terremoto) < 0) {
            no->direita = rotacaoDireita(no->direita);
            return rotacaoEsquerda(no);
        }

        return no;
    }

    // Função auxiliar para inserir um terremoto na árvore
    NoAVL* inserirAux(NoAVL* no, const Terremoto& t) {
        // Inserção normal em BST
        if (no == nullptr)
            return new NoAVL(t);

        // Decide qual subárvore inserir com base na data
        if (compararTerremotos(t, no->terremoto) < 0)
            no->esquerda = inserirAux(no->esquerda, t);
        else if (compararTerremotos(t, no->terremoto) > 0)
            no->direita = inserirAux(no->direita, t);
        else
            return no; // Duplicatas não são permitidas

        // Balanceia a árvore
        return balancear(no, t);
    }

    // Função para encontrar o nó com valor mínimo
    NoAVL* encontrarMinimo(NoAVL* no) {
        NoAVL* atual = no;
        while (atual && atual->esquerda != nullptr)
            atual = atual->esquerda;
        return atual;
    }

    // Função auxiliar para remover um terremoto da árvore
    NoAVL* removerAux(NoAVL* no, const Terremoto& t) {
        if (no == nullptr)
            return nullptr;

        // Localiza o nó a ser removido
        if (compararTerremotos(t, no->terremoto) < 0)
            no->esquerda = removerAux(no->esquerda, t);
        else if (compararTerremotos(t, no->terremoto) > 0)
            no->direita = removerAux(no->direita, t);
        else {
            // Nó com um ou nenhum filho
            if (no->esquerda == nullptr) {
                NoAVL* temp = no->direita;
                delete no;
                return temp;
            } else if (no->direita == nullptr) {
                NoAVL* temp = no->esquerda;
                delete no;
                return temp;
            }

            // Nó com dois filhos
            NoAVL* temp = encontrarMinimo(no->direita);
            no->terremoto = temp->terremoto;
            no->direita = removerAux(no->direita, temp->terremoto);
        }

        // Se a árvore tinha apenas um nó
        if (no == nullptr)
            return nullptr;

        // Atualiza a altura e balanceia
        atualizarAltura(no);
        int fator = getFatorBalanceamento(no);

        // Casos de balanceamento após remoção
        // Caso Esquerda-Esquerda
        if (fator > 1 && getFatorBalanceamento(no->esquerda) >= 0)
            return rotacaoDireita(no);

        // Caso Esquerda-Direita
        if (fator > 1 && getFatorBalanceamento(no->esquerda) < 0) {
            no->esquerda = rotacaoEsquerda(no->esquerda);
            return rotacaoDireita(no);
        }

        // Caso Direita-Direita
        if (fator < -1 && getFatorBalanceamento(no->direita) <= 0)
            return rotacaoEsquerda(no);

        // Caso Direita-Esquerda
        if (fator < -1 && getFatorBalanceamento(no->direita) > 0) {
            no->direita = rotacaoDireita(no->direita);
            return rotacaoEsquerda(no);
        }

        return no;
    }

    // Função para comparar dois terremotos (ordenação por data e hora)
    int compararTerremotos(const Terremoto& t1, const Terremoto& t2) const {
        // Primeiro compara por data
        int compData = t1.data.compare(t2.data);
        if (compData != 0) return compData;
        
        // Se as datas forem iguais, compara por hora
        int compHora = t1.hora.compare(t2.hora);
        if (compHora != 0) return compHora;
        
        // Se data e hora forem iguais, compara por magnitude (ordem decrescente)
        if (t1.magnitude > t2.magnitude) return -1;
        if (t1.magnitude < t2.magnitude) return 1;
        
        // Se tudo for igual, compara por país e cidade
        int compPais = t1.pais.compare(t2.pais);
        if (compPais != 0) return compPais;
        
        return t1.cidade.compare(t2.cidade);
    }

    // Função auxiliar para buscar terremotos por data
    void buscarPorDataAux(NoAVL* no, const std::string& data, std::vector<Terremoto>& resultados) {
        if (no == nullptr) return;

        // Busca na subárvore esquerda
        buscarPorDataAux(no->esquerda, data, resultados);

        // Verifica o nó atual
        if (no->terremoto.data == data) {
            resultados.push_back(no->terremoto);
        }

        // Busca na subárvore direita
        buscarPorDataAux(no->direita, data, resultados);
    }

    // Função auxiliar para buscar terremotos por país
    void buscarPorPaisAux(NoAVL* no, const std::string& pais, std::vector<Terremoto>& resultados) {
        if (no == nullptr) return;

        // Busca na subárvore esquerda
        buscarPorPaisAux(no->esquerda, pais, resultados);

        // Verifica o nó atual
        if (no->terremoto.pais == pais) {
            resultados.push_back(no->terremoto);
        }

        // Busca na subárvore direita
        buscarPorPaisAux(no->direita, pais, resultados);
    }

    // Função auxiliar para buscar terremotos por magnitude
    void buscarPorMagnitudeAux(NoAVL* no, double minMag, double maxMag, std::vector<Terremoto>& resultados) {
        if (no == nullptr) return;

        // Busca na subárvore esquerda
        buscarPorMagnitudeAux(no->esquerda, minMag, maxMag, resultados);

        // Verifica o nó atual
        if (no->terremoto.magnitude >= minMag && no->terremoto.magnitude <= maxMag) {
            resultados.push_back(no->terremoto);
        }

        // Busca na subárvore direita
        buscarPorMagnitudeAux(no->direita, minMag, maxMag, resultados);
    }

    // Função auxiliar para buscar terremotos por classificação de risco
    void buscarPorRiscoAux(NoAVL* no, const std::string& risco, std::vector<Terremoto>& resultados) {
        if (no == nullptr) return;

        // Busca na subárvore esquerda
        buscarPorRiscoAux(no->esquerda, risco, resultados);

        // Verifica o nó atual
        if (no->terremoto.classificarRisco() == risco) {
            resultados.push_back(no->terremoto);
        }

        // Busca na subárvore direita
        buscarPorRiscoAux(no->direita, risco, resultados);
    }

    // Função auxiliar para liberar a memória da árvore
    void liberarMemoriaAux(NoAVL* no) {
        if (no == nullptr) return;

        liberarMemoriaAux(no->esquerda);
        liberarMemoriaAux(no->direita);
        delete no;
    }

public:
    // Construtor
    ArvoreAVL() : raiz(nullptr) {}

    // Destrutor
    ~ArvoreAVL() {
        liberarMemoria();
    }

    // Função para inserir um terremoto na árvore
    void inserir(const Terremoto& t) {
        raiz = inserirAux(raiz, t);
    }

    // Função para remover um terremoto da árvore
    void remover(const Terremoto& t) {
        raiz = removerAux(raiz, t);
    }

    // Função para buscar terremotos por data
    std::vector<Terremoto> buscarPorData(const std::string& data) {
        std::vector<Terremoto> resultados;
        buscarPorDataAux(raiz, data, resultados);
        return resultados;
    }

    // Função para buscar terremotos por país
    std::vector<Terremoto> buscarPorPais(const std::string& pais) {
        std::vector<Terremoto> resultados;
        buscarPorPaisAux(raiz, pais, resultados);
        return resultados;
    }

    // Função para buscar terremotos por magnitude
    std::vector<Terremoto> buscarPorMagnitude(double minMag, double maxMag) {
        std::vector<Terremoto> resultados;
        buscarPorMagnitudeAux(raiz, minMag, maxMag, resultados);
        return resultados;
    }

    // Função para buscar terremotos por classificação de risco
    std::vector<Terremoto> buscarPorRisco(const std::string& risco) {
        std::vector<Terremoto> resultados;
        buscarPorRiscoAux(raiz, risco, resultados);
        return resultados;
    }

    // Função para liberar a memória da árvore
    void liberarMemoria() {
        liberarMemoriaAux(raiz);
        raiz = nullptr;
    }

    // Função para carregar dados de um arquivo CSV - MODIFICADA
    bool carregarDados(const std::string& nomeArquivo) {
        std::ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) {
            std::cerr << "Erro ao abrir o arquivo: " << nomeArquivo << std::endl;
            return false;
        }

        std::cout << "Carregando dados do arquivo..." << std::endl;
        
        std::string linha;
        // Pula a linha de cabeçalho
        std::getline(arquivo, linha);

        int contador = 0;
        int progresso = 0;
        
        // Conta o número total de linhas para mostrar progresso
        std::ifstream contadorArquivo(nomeArquivo);
        int totalLinhas = 0;
        while (std::getline(contadorArquivo, linha)) {
            totalLinhas++;
        }
        contadorArquivo.close();
        
        // Volta ao início do arquivo
        arquivo.clear();
        arquivo.seekg(0);
        std::getline(arquivo, linha); // Pula o cabeçalho novamente
        
        while (std::getline(arquivo, linha)) {
            std::stringstream ss(linha);
            std::string data, hora, cidade, pais;
            double latitude, longitude, magnitude, profundidade, impacto;
            std::string token;

            // Lê cada campo separado por vírgula
            std::getline(ss, data, ',');
            std::getline(ss, hora, ',');
            std::getline(ss, cidade, ',');
            std::getline(ss, pais, ',');
            
            std::getline(ss, token, ',');
            try {
                latitude = std::stod(token);
            } catch (const std::exception& e) {
                latitude = 0.0;
            }
            
            std::getline(ss, token, ',');
            try {
                longitude = std::stod(token);
            } catch (const std::exception& e) {
                longitude = 0.0;
            }
            
            std::getline(ss, token, ',');
            try {
                magnitude = std::stod(token);
            } catch (const std::exception& e) {
                magnitude = 0.0;
            }
            
            std::getline(ss, token, ',');
            try {
                profundidade = std::stod(token);
            } catch (const std::exception& e) {
                profundidade = 0.0;
            }
            
            std::getline(ss, token, ',');
            try {
                impacto = std::stod(token);
            } catch (const std::exception& e) {
                impacto = 0.0;
            }

            // Cria um novo terremoto e insere na árvore
            Terremoto t(data, hora, cidade, pais, latitude, longitude, magnitude, profundidade, impacto);
            raiz = inserirAux(raiz, t);
            contador++;
            
            // Atualiza o progresso a cada 10% dos dados carregados
            int novoProgresso = (contador * 100) / (totalLinhas - 1);
            if (novoProgresso >= progresso + 10) {
                progresso = novoProgresso;
                std::cout << "Progresso: " << progresso << "%" << std::endl;
            }
        }
        
        arquivo.close();
        
        std::cout << "Total de " << contador << " terremotos carregados com sucesso!" << std::endl;
        return true;
    }

    // Função para registrar um terremoto atual
    Terremoto registrarTerremotoAtual(const std::string& cidade, const std::string& pais, double magnitude) {
        // Obtém a data e hora atual
        std::time_t agora = std::time(nullptr);
        std::tm* tempoLocal = std::localtime(&agora);
        
        std::stringstream dataStr, horaStr;
        dataStr << std::put_time(tempoLocal, "%Y-%m-%d");
        horaStr << std::put_time(tempoLocal, "%H:%M:%S");
        
        // Valores padrão para latitude, longitude e profundidade
        double latitude = 0.0;
        double longitude = 0.0;
        double profundidade = 10.0; // Profundidade padrão de 10 km
        
        // Calcula o impacto com base na magnitude
        double impacto = magnitude * 1.2;
        if (impacto > 10) impacto = 10;
        
        Terremoto novoTerremoto(dataStr.str(), horaStr.str(), cidade, pais, 
                               latitude, longitude, magnitude, profundidade, impacto);
        
        // Insere o novo terremoto na árvore
        inserir(novoTerremoto);
        
        return novoTerremoto;
    }

    // Função para emitir alerta com base no terremoto
    std::string emitirAlerta(const Terremoto& t) {
        std::string nivelAlerta;
        std::string mensagem;
        
        // Determina o nível de alerta com base na magnitude e impacto
        if (t.magnitude >= 7.0 || t.impacto >= 8.0) {
            nivelAlerta = "ALTO";
            mensagem = "ALERTA CRÍTICO: Terremoto de grande magnitude detectado!";
        } else if (t.magnitude >= 5.0 || t.impacto >= 5.0) {
            nivelAlerta = "MÉDIO";
            mensagem = "ALERTA: Terremoto significativo detectado.";
        } else {
            nivelAlerta = "BAIXO";
            mensagem = "Aviso: Terremoto de baixa intensidade registrado.";
        }
        
        std::stringstream alerta;
        alerta << "=================================================\n";
        alerta << "ALERTA DE TERREMOTO - NÍVEL: " << nivelAlerta << "\n";
        alerta << mensagem << "\n";
        alerta << "Data/Hora: " << t.data << " " << t.hora << "\n";
        alerta << "Local: " << t.cidade << ", " << t.pais << "\n";
        alerta << "Magnitude: " << t.magnitude << "\n";
        alerta << "Impacto estimado: " << t.impacto << "/10\n";
        
        // Adiciona recomendações com base no nível de alerta
        if (nivelAlerta == "ALTO") {
            alerta << "\nRECOMENDAÇÕES:\n";
            alerta << "- Evacuar áreas próximas ao epicentro\n";
            alerta << "- Ativar protocolos de emergência\n";
            alerta << "- Monitorar possíveis tsunamis se próximo à costa\n";
        } else if (nivelAlerta == "MÉDIO") {
            alerta << "\nRECOMENDAÇÕES:\n";
            alerta << "- Verificar danos estruturais em edificações\n";
            alerta << "- Preparar equipes de resgate\n";
            alerta << "- Monitorar réplicas\n";
        }
        
        alerta << "=================================================\n";
        
        return alerta.str();
    }
};

// Função principal - única função main no código
int main() {
    // Caminho do arquivo fornecido
    std::string caminhoArquivo = "C:/Users/letic/OneDrive/Documentos/Repositorio-github/Estrutura-de-Dados/earthquake_dataset.csv";
    
    int opcaoEstrutura = 0;
    bool executando = true;
    
    std::cout << "===== SISTEMA DE MONITORAMENTO DE TERREMOTOS =====\n";
    std::cout << "Escolha a estrutura de dados a ser utilizada:\n";
    std::cout << "1. Árvore AVL\n";
    std::cout << "2. Lista Encadeada (não implementada)\n";
    std::cout << "3. Tabela Hash (não implementada)\n";
    std::cout << "4. Skip List (não implementada)\n";
    std::cout << "5. KD Tree (não implementada)\n";
    std::cout << "Escolha uma opção: ";
    std::cin >> opcaoEstrutura;
    
    // Por enquanto, apenas a árvore AVL está implementada
    if (opcaoEstrutura != 1) {
        std::cout << "Estrutura de dados ainda não implementada. Utilizando Árvore AVL por padrão.\n";
        opcaoEstrutura = 1;
    }
    
    // Inicializa a estrutura de dados escolhida
    ArvoreAVL arvore;
    
    std::cout << "Carregando dados do arquivo: " << caminhoArquivo << std::endl;
    if (!arvore.carregarDados(caminhoArquivo)) {
        std::cerr << "Falha ao carregar os dados. Verifique o arquivo." << std::endl;
        return 1;
    }
    
    std::cout << "Dados carregados com sucesso!\n";
    
    // Limpa o buffer de entrada e aguarda uma tecla para continuar
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Pressione Enter para continuar...";
    std::cin.get();
    
    std::cout << "\nIniciando menu de opções...\n";
    
    while (executando) {
        // Limpa a tela (opcional - funciona em sistemas Windows)
        // system("cls");  // Descomente se estiver no Windows
        // system("clear");  // Descomente se estiver no Linux/Mac
        
        int opcao = 0;
        std::cout << "\n===== SISTEMA DE MONITORAMENTO DE TERREMOTOS =====\n";
        std::cout << "1. Registrar um novo terremoto\n";
        std::cout << "2. Busca de terremotos\n";
        std::cout << "3. Registrar um terremoto que está ocorrendo agora\n";
        std::cout << "4. Sair\n";
        std::cout << "Escolha uma opção: ";
        std::cin >> opcao;
        
        switch (opcao) {
            case 1: {
                // Registrar um novo terremoto
                std::string data, hora, cidade, pais;
                double latitude, longitude, magnitude, profundidade, impacto;
                
                std::cin.ignore(); // Limpa o buffer do teclado
                
                std::cout << "Data (AAAA-MM-DD): ";
                std::getline(std::cin, data);
                
                std::cout << "Hora (HH:MM:SS): ";
                std::getline(std::cin, hora);
                
                std::cout << "Cidade: ";
                std::getline(std::cin, cidade);
                
                std::cout << "País: ";
                std::getline(std::cin, pais);
                
                std::cout << "Latitude: ";
                std::cin >> latitude;
                
                std::cout << "Longitude: ";
                std::cin >> longitude;
                
                std::cout << "Magnitude: ";
                std::cin >> magnitude;
                
                std::cout << "Profundidade (km): ";
                std::cin >> profundidade;
                
                std::cout << "Impacto (0-10): ";
                std::cin >> impacto;
                
                Terremoto novoTerremoto(data, hora, cidade, pais, latitude, longitude, magnitude, profundidade, impacto);
                arvore.inserir(novoTerremoto);
                
                std::cout << "\nTerremoto registrado com sucesso!\n";
                novoTerremoto.exibir();
                
                // Aguarda o usuário pressionar Enter para continuar
                std::cin.ignore();
                std::cout << "Pressione Enter para continuar...";
                std::cin.get();
                break;
            }
            case 2: {
                // Busca de terremotos
                int tipoBusca;
                std::cout << "\n----- BUSCA DE TERREMOTOS -----\n";
                std::cout << "1. Buscar por data\n";
                std::cout << "2. Buscar por país\n";
                std::cout << "3. Buscar por magnitude\n";
                std::cout << "4. Buscar por classificação de risco\n";
                std::cout << "Escolha uma opção: ";
                std::cin >> tipoBusca;
                
                std::vector<Terremoto> resultados;
                
                switch (tipoBusca) {
                    case 1: {
                        std::string data;
                        std::cin.ignore();
                        std::cout << "Digite a data (AAAA-MM-DD): ";
                        std::getline(std::cin, data);
                        resultados = arvore.buscarPorData(data);
                        break;
                    }
                    case 2: {
                        std::string pais;
                        std::cin.ignore();
                        std::cout << "Digite o país: ";
                        std::getline(std::cin, pais);
                        resultados = arvore.buscarPorPais(pais);
                        break;
                    }
                    case 3: {
                        double minMag, maxMag;
                        std::cout << "Digite a magnitude mínima: ";
                        std::cin >> minMag;
                        std::cout << "Digite a magnitude máxima: ";
                        std::cin >> maxMag;
                        resultados = arvore.buscarPorMagnitude(minMag, maxMag);
                        break;
                    }
                    case 4: {
                        std::string risco;
                        std::cin.ignore();
                        std::