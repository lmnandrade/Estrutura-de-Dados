#ifndef ALERTA_H
#define ALERTA_H

#include <string>
#include <vector>
#include <map>
#include "csv_reader.h"      // Para EarthquakeRecord
#include "classification.h"  // Para CountryRiskProfile

// Declaração da função
void alerta_terremoto(
    const std::map<std::string, CountryRiskProfile>& country_risks,
    const std::vector<std::string>& registered_emails,
    const std::vector<EarthquakeRecord>& all_raw_records
);

#endif // ALERTA_H