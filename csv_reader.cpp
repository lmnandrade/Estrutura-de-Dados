#include "csv_reader.h"
#include <fstream>   // Para std::ifstream
#include <sstream>   // Para std::stringstream
#include <stdexcept> // Para std::runtime_error
#include <iomanip>   // Para std::setw (se quiser usar na displayRecord, mas vou fazer simples)

// Implementação da função para ler o arquivo CSV
std::vector<EarthquakeRecord> read_earthquake_csv(const std::string& filename) {
    std::vector<EarthquakeRecord> records;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ERRO: Nao foi possivel abrir o arquivo: " << filename << std::endl;
        return records; // Retorna vetor vazio
    }

    std::string line;
    int line_number = 0;

    // Pular a linha do cabeçalho
    if (std::getline(file, line)) {
        line_number++; // Contabiliza a linha do cabeçalho
    } else {
        std::cerr << "ERRO: Arquivo CSV vazio ou nao foi possivel ler o cabecalho." << std::endl;
        file.close();
        return records; // Retorna vetor vazio
    }

    while (std::getline(file, line)) {
        line_number++;
        std::stringstream ss(line);
        std::string field;
        EarthquakeRecord record;
        // int field_index = 0; // Removido pois não é mais usado diretamente no tratamento de erro de conversão

        try {
            // 0: Date
            if (std::getline(ss, field, ',')) record.date = field;
            else throw std::runtime_error("Campo 'Date' ausente.");
            // field_index++;

            // 1: Time (UTC)
            if (std::getline(ss, field, ',')) record.time = field;
            else throw std::runtime_error("Campo 'Time (UTC)' ausente.");
            // field_index++;

            // 2: City
            if (std::getline(ss, field, ',')) record.city = field;
            else throw std::runtime_error("Campo 'City' ausente.");
            // field_index++;
            
            // 3: Country
            if (std::getline(ss, field, ',')) record.country = field;
            else throw std::runtime_error("Campo 'Country' ausente.");
            // field_index++;

            // 4: Latitude (lido como string)
            if (std::getline(ss, field, ',')) record.latitude = field;
            else throw std::runtime_error("Campo 'Latitude' ausente.");
            // field_index++;

            // 5: Longitude (lido como string)
            if (std::getline(ss, field, ',')) record.longitude = field;
            else throw std::runtime_error("Campo 'Longitude' ausente.");
            // field_index++;

            // 6: Earthquake Magnitude (lido como string)
            if (std::getline(ss, field, ',')) record.magnitude = field;
            else throw std::runtime_error("Campo 'Earthquake Magnitude' ausente.");
            // field_index++;

            // 7: Depth (km) (lido como string)
            if (std::getline(ss, field, ',')) record.depth = field;
            else throw std::runtime_error("Campo 'Depth (km)' ausente.");
            // field_index++;

            // 8: Impact Score (lido como string)
            // Este é o último campo, não precisa de ',' no final do getline
            if (std::getline(ss, field)) { // Lê o resto da linha para o último campo
                 // Remove espaços em branco no início/fim do campo, se houver (opcional, mas bom)
                field.erase(0, field.find_first_not_of(" \t\n\r\f\v"));
                field.erase(field.find_last_not_of(" \t\n\r\f\v") + 1);
                record.impact_score = field;
            } else throw std::runtime_error("Campo 'Impact Score' ausente.");
            // field_index++;
            
            records.push_back(record);

        } catch (const std::runtime_error& re) {
            std::cerr << "AVISO: Erro de formato na linha " << line_number << ". Erro: " << re.what() 
                      << ". Linha ignorada: " << line << std::endl;
            // Pula este registro
        }
    }

    file.close();
    return records;
}

// Implementação da função para exibir um único registro de terremoto
void displayRecord(const EarthquakeRecord& record) {
    // Saída simples, você pode formatar melhor com <iomanip> se desejar
    std::cout << "  Data: " << record.date
              << ", Hora UTC: " << record.time
              << ", Cidade: " << record.city
              << ", Pais: " << record.country
              << ", Lat: " << record.latitude
              << ", Lon: " << record.longitude
              << ", Mag: " << record.magnitude
              << ", Prof (km): " << record.depth
              << ", Impact Score: " << record.impact_score << std::endl;
}
void save_earthquake_csv(const std::string& filename, const std::vector<EarthquakeRecord>& records) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERRO: Nao foi possivel abrir o arquivo para escrita: " << filename << std::endl;
        return;
    }
    // Escreve o cabeçalho
    file << "Date,Time (UTC),City,Country,Latitude,Longitude,Earthquake Magnitude,Depth (km),Impact Score\n";
    for (const auto& rec : records) {
        file << rec.date << "," << rec.time << "," << rec.city << "," << rec.country << ","
             << rec.latitude << "," << rec.longitude << "," << rec.magnitude << ","
             << rec.depth << "," << rec.impact_score << "\n";
    }
    file.close();
}