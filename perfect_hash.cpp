#include "perfect_hash.h"
#include <iostream>
#include <iomanip>
#include <algorithm> // For std::transform, std::find_if
#include <map>       // For grouping during build

// Assuming KDTreeHelpers is available, e.g., from a shared utility header or defined above
// If not, you'd need to include its definition here.
// For this example, we assume it's linked or included via perfect_hash.h's includes.
// Example:
/*
namespace KDTreeHelpers {
    bool try_string_to_double(const std::string& s, double& out, const std::string& field_name_for_error, bool suppress_error_message) {
        try {
            out = std::stod(s);
            return true;
        } catch (const std::invalid_argument&) {
            if (!suppress_error_message) std::cerr << "ERRO: Valor invalido para " << field_name_for_error << ": '" << s << "'." << std::endl;
            return false;
        } catch (const std::out_of_range&) {
            if (!suppress_error_message) std::cerr << "ERRO: Valor fora do intervalo para " << field_name_for_error << ": '" << s << "'." << std::endl;
            return false;
        }
    }
}
*/


// --- Private Helper Functions ---

std::string PerfectHashTable::generate_key(const EarthquakeRecord& record) const {
    return record.date + "_" + record.time + "_" + record.city;
}

std::string PerfectHashTable::generate_key(const std::string& date, const std::string& time_utc, const std::string& city) const {
    return date + "_" + time_utc + "_" + city;
}

size_t PerfectHashTable::h_primary(const std::string& key) const {
    if (primary_table.empty()) return 0; // Should not happen if built
    return std::hash<std::string>{}(key) % primary_table.size();
}

// A simple universal hash for secondary level. (key_hash + seed) % mod_val
// More sophisticated universal hash functions exist.
size_t PerfectHashTable::h_secondary(const std::string& key, size_t seed_val, size_t mod_val) const {
    if (mod_val == 0) return 0; // Avoid division by zero
    // Combine key's hash with seed for more variability
    // Using a slightly more robust combination than simple addition
    std::hash<std::string> str_hasher;
    size_t key_hash = str_hasher(key);
    
    // Using a common mixing function (from MurmurHash, simplified)
    key_hash ^= seed_val;
    key_hash *= 0x85ebca6b;
    key_hash ^= key_hash >> 13;
    key_hash *= 0xc2b2ae35;
    key_hash ^= key_hash >> 16;

    return key_hash % mod_val;
}


// --- Constructor ---
PerfectHashTable::PerfectHashTable(size_t expected_elements) : num_elements(0) {
    // Primary table size can be N (number of elements) for FKS,
    // or a bit larger to reduce primary collisions.
    // For now, initialize primary_table_target_size. Build will actually size it.
    primary_table_target_size = expected_elements > 0 ? expected_elements : 100;
}

// --- Core Build Method ---
void PerfectHashTable::build_table(const std::vector<EarthquakeRecord>& records) {
    num_elements = 0;
    if (records.empty()) {
        primary_table.clear();
        // std::cout << "INFO (PerfectHash): No records to build. Table is empty." << std::endl;
        return;
    }

    // Make a mutable copy to handle potential duplicates by key
    // Or ensure records are unique by key before this step
    std::vector<EarthquakeRecord> unique_records;
    std::map<std::string, EarthquakeRecord> temp_map;
    for(const auto& rec : records) {
        temp_map[generate_key(rec)] = rec; // Last one wins for duplicate keys
    }
    for(const auto& pair : temp_map) {
        unique_records.push_back(pair.second);
    }
    
    size_t N = unique_records.size();
    primary_table.assign(N, SecondaryHashTable()); // Level 1 table size N

    // Stage 1: Distribute records into primary buckets
    std::vector<std::vector<EarthquakeRecord>> primary_buckets(N);
    for (const auto& record : unique_records) {
        std::string key = generate_key(record);
        size_t primary_idx = h_primary(key);
        primary_buckets[primary_idx].push_back(record);
    }

    // Stage 2: For each primary bucket, find a collision-free secondary hash
    for (size_t i = 0; i < N; ++i) {
        const auto& bucket_records = primary_buckets[i];
        size_t n_i = bucket_records.size();

        if (n_i == 0) continue;

        primary_table[i].size = n_i * n_i; // Secondary table size n_i^2
        if (primary_table[i].size == 0 && n_i > 0) { // Case for n_i = 1, size becomes 1
             primary_table[i].size = 1;
        }
        primary_table[i].slots.assign(primary_table[i].size, HashNodePerfect()); // Initialize with invalid nodes
        primary_table[i].seed = 1; // Start with seed 1

        bool collision_free_found = false;
        while (!collision_free_found) {
            if (primary_table[i].seed > 1000 * n_i && n_i >1) { // Safety break for too many seed attempts
                // This indicates an issue with hash function or extreme bad luck.
                // Could fall back to a list for this bucket or error out.
                // std::cerr << "ERRO (PerfectHash): Could not find collision-free secondary hash for bucket " << i 
                //           << " with " << n_i << " elements after many attempts. Key example: " << generate_key(bucket_records[0]) << std::endl;
                // Fallback: make it a single slot and just store the first one if n_i is small or handle error
                // For simplicity, we'll proceed, but it might not be perfect for this bucket.
                // Or, resize secondary and try again.
                // A more robust solution would increase secondary_table[i].size and reset seed.
                // For now, let's just try a much larger seed.
                primary_table[i].seed = (std::hash<size_t>{}(i) % 100000) + 1001 * n_i ; // Try a very different seed
                if (primary_table[i].seed > 2000 * n_i && n_i >1) { // Final safety break
                    // std::cerr << "ERRO FATAL (PerfectHash): Still cannot find collision-free. Bucket " << i << " will have collisions." << std::endl;
                    // To avoid infinite loop, mark as found and accept collisions for this specific bucket
                    // Or throw an exception
                    // For this example, we'll proceed and it won't be "perfect" for this bucket.
                    collision_free_found = true; // Force exit
                     primary_table[i].slots.assign(n_i, HashNodePerfect()); // Fallback to n_i size
                     for(size_t k=0; k < n_i; ++k) { // Linear probing like simple fallback
                        if(k < primary_table[i].slots.size()){
                           primary_table[i].slots[k] = HashNodePerfect(generate_key(bucket_records[k]), bucket_records[k]);
                           num_elements++;
                        }
                     }
                    continue; // Move to next primary bucket
                }
            }

            // Clear previous attempt for this seed
            primary_table[i].slots.assign(primary_table[i].size, HashNodePerfect());
            bool current_seed_ok = true;
            size_t temp_elements_for_bucket = 0;

            for (const auto& record : bucket_records) {
                std::string key = generate_key(record);
                size_t secondary_idx = h_secondary(key, primary_table[i].seed, primary_table[i].size);
                
                if (primary_table[i].slots[secondary_idx].is_valid) { // Collision!
                    current_seed_ok = false;
                    primary_table[i].seed++;
                    break; 
                }
                primary_table[i].slots[secondary_idx] = HashNodePerfect(key, record);
                temp_elements_for_bucket++;
            }

            if (current_seed_ok) {
                collision_free_found = true;
                num_elements += temp_elements_for_bucket;
            }
        }
    }
    // std::cout << "INFO (PerfectHash): Build complete. Total elements: " << num_elements << std::endl;
}


// --- Public Operations ---

void PerfectHashTable::insert_record(const EarthquakeRecord& record) {
    // WARNING: True perfect hashing is for static sets.
    // Inserting after build can break perfectness if not handled carefully
    // (e.g., by rebuilding the specific secondary table if a collision occurs).
    // This is a simplified insertion.
    if (primary_table.empty()) {
        // std::cout << "AVISO (PerfectHash): Table not built. Building with this single record." << std::endl;
        build_table({record}); // Build if not already built
        return;
    }

    std::string key = generate_key(record);
    size_t primary_idx = h_primary(key);

    // Check if already exists (important to avoid increasing num_elements for duplicates)
    const EarthquakeRecord* existing = search_record(record.date, record.time, record.city);
    if (existing) {
        // std::cout << "AVISO (PerfectHash): Record with key '" << key << "' already exists. Not inserting." << std::endl;
        return;
    }
    
    SecondaryHashTable& sec_table = primary_table[primary_idx];
    if (sec_table.size == 0) { // Unlikely if built, but handle if primary bucket was empty
        // std::cout << "AVISO (PerfectHash): Inserting into an uninitialized secondary table for primary_idx " << primary_idx <<". Rebuilding this small bucket." << std::endl;
        // This indicates a new primary slot effectively. We need to initialize its secondary table.
        // Simplest is to rebuild with the new record included in that bucket's items.
        // For this example, this part is non-trivial and can be complex.
        // A proper implementation would collect all items for this primary_idx (old + new)
        // and re-run the secondary hash finding for this bucket.
        // For now, let's do a very naive add which might collide.
        sec_table.size = 1; // Minimal size
        sec_table.slots.assign(1, HashNodePerfect());
        sec_table.seed = 1; // Default seed
    }

    size_t secondary_idx = h_secondary(key, sec_table.seed, sec_table.size);

    if (sec_table.slots[secondary_idx].is_valid) {
        // Collision in a supposedly "perfect" secondary table!
        // This means either a new key hashes to an existing slot, or hash conflict.
        // A robust solution would resize sec_table.slots (e.g., (current_elements_in_bucket+1)^2),
        // copy existing elements, add the new one, and then re-find a collision-free seed.
        // std::cerr << "ERRO/AVISO (PerfectHash): Collision on insert into secondary table for key '" << key 
        //           << "'. Perfectness might be lost for this bucket or requires rebuild of bucket." << std::endl;
        // Simplistic: try to find an empty slot (linear probing, makes it not perfect)
        bool placed = false;
        for(size_t i = 0; i < sec_table.size; ++i) {
            size_t try_idx = (secondary_idx + i) % sec_table.size;
            if (!sec_table.slots[try_idx].is_valid) {
                sec_table.slots[try_idx] = HashNodePerfect(key, record);
                num_elements++;
                placed = true;
                break;
            }
        }
        if (!placed) {
            // std::cerr << "ERRO (PerfectHash): Bucket full for key '" << key << "' on insert. Record not added." << std::endl;
        }

    } else {
        sec_table.slots[secondary_idx] = HashNodePerfect(key, record);
        num_elements++;
    }
}

const EarthquakeRecord* PerfectHashTable::search_record(const std::string& date, const std::string& time_utc, const std::string& city) const {
    if (primary_table.empty() || num_elements == 0) return nullptr;

    std::string key = generate_key(date, time_utc, city);
    size_t primary_idx = h_primary(key);

    if (primary_idx >= primary_table.size()) return nullptr; // Should not happen

    const SecondaryHashTable& sec_table = primary_table[primary_idx];
    if (sec_table.size == 0) return nullptr; // No secondary table for this primary slot

    size_t secondary_idx = h_secondary(key, sec_table.seed, sec_table.size);
    
    if (secondary_idx >= sec_table.slots.size()) return nullptr; // Should not happen

    const HashNodePerfect& node = sec_table.slots[secondary_idx];

    // Must check key equality because h_secondary maps to an index,
    // but the item at that index might be from a different key if there was a
    // fallback during build or a non-perfect insert.
    // Also, the key in the node must match the search key.
    if (node.is_valid && node.key == key) {
        return &(node.record);
    }
    
    // If a collision resolution (like linear probing) was used on insert/build-fallback for this bucket,
    // we might need to probe here too. This example's search assumes direct hit or miss.
    // Fallback: If not found at the computed index, search all slots for the key (handles non-perfect fallback cases)
    for(const auto& n_fallback : sec_table.slots){
        if(n_fallback.is_valid && n_fallback.key == key) return &(n_fallback.record);
    }


    return nullptr;
}

bool PerfectHashTable::remove_record(const std::string& date, const std::string& time_utc, const std::string& city) {
    if (primary_table.empty() || num_elements == 0) return false;

    std::string key = generate_key(date, time_utc, city);
    size_t primary_idx = h_primary(key);

    if (primary_idx >= primary_table.size()) return false;

    SecondaryHashTable& sec_table = primary_table[primary_idx]; // Mutable for is_valid
    if (sec_table.size == 0) return false;

    size_t secondary_idx = h_secondary(key, sec_table.seed, sec_table.size);

    if (secondary_idx >= sec_table.slots.size()) return false;
    
    HashNodePerfect& node = sec_table.slots[secondary_idx];

    if (node.is_valid && node.key == key) {
        node.is_valid = false; // Mark as deleted
        num_elements--;
        return true;
    }
    
    // Fallback: If not found at the computed index, search all slots for the key (handles non-perfect fallback cases)
    for(auto& n_fallback : sec_table.slots){ // auto& for mutable
        if(n_fallback.is_valid && n_fallback.key == key) {
            n_fallback.is_valid = false;
            num_elements--;
            return true;
        }
    }

    return false;
}

int PerfectHashTable::get_count() const {
    return num_elements;
}

bool PerfectHashTable::is_empty() const {
    return num_elements == 0;
}


// --- Utility and Listing Functions ---

void PerfectHashTable::display_table_structure() const {
    std::cout << "\n--- Perfect Hash Table Structure ---" << std::endl;
    std::cout << "Total Elements: " << num_elements << std::endl;
    if (primary_table.empty()) {
        std::cout << "Primary table is empty." << std::endl;
        return;
    }
    std::cout << "Primary Table Size: " << primary_table.size() << std::endl;

    for (size_t i = 0; i < primary_table.size(); ++i) {
        const auto& sec_table = primary_table[i];
        int valid_in_bucket = 0;
        for(const auto& node : sec_table.slots) if(node.is_valid) valid_in_bucket++;

        if (sec_table.size > 0 || valid_in_bucket > 0) { // Only show if secondary table exists or has elements
            std::cout << "Primary Idx [" << std::setw(3) << i << "]: "
                      << "Sec.Size=" << sec_table.size
                      << ", Sec.Seed=" << sec_table.seed
                      << ", ValidElements=" << valid_in_bucket << std::endl;
            if (valid_in_bucket > 0 && valid_in_bucket < 10) { // Print details if few elements
                for(size_t j=0; j < sec_table.slots.size(); ++j) {
                    if (sec_table.slots[j].is_valid) {
                        std::cout << "    Sec.Idx [" << std::setw(2) << j << "]: Key='" << sec_table.slots[j].key.substr(0,20) << "...'" << std::endl;
                    }
                }
            }
        }
    }
    std::cout << "------------------------------------" << std::endl;
}

std::vector<EarthquakeRecord> PerfectHashTable::get_all_records_vector() const {
    std::vector<EarthquakeRecord> all_records;
    all_records.reserve(num_elements);
    for (const auto& sec_hash_table_info : primary_table) {
        for (const auto& node : sec_hash_table_info.slots) {
            if (node.is_valid) {
                all_records.push_back(node.record);
            }
        }
    }
    return all_records;
}

void PerfectHashTable::list_all_records() const {
    if (is_empty()) {
        std::cout << "Perfect Hash Table is empty." << std::endl;
        return;
    }
    std::cout << "\n--- All Records in Perfect Hash Table (" << num_elements << ") ---" << std::endl;
    int count = 1;
    for (const auto& sec_hash_table_info : primary_table) {
        for (const auto& node : sec_hash_table_info.slots) {
            if (node.is_valid) {
                std::cout << "Record #" << count++ << " (Key: "" << node.key << ""):" << std::endl;
                std::cout << "  ";
                displayRecord(node.record); // Assumes displayRecord is available
                 if (num_elements > 1 && static_cast<size_t>(count) <= num_elements) {
                     std::cout << "  --------------------------------------------------------------------------------" << std::endl;
                }
            }
        }
    }
     if (num_elements > 0) {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}

void PerfectHashTable::list_by_min_magnitude(const std::string& min_mag_str) const {
    if (is_empty()) {
        std::cout << "Perfect Hash Table is empty. Nothing to list." << std::endl;
        return;
    }
    double min_mag_val;
    if (!KDTreeHelpers::try_string_to_double(min_mag_str, min_mag_val, "Magnitude Minima", false)) {
        return;
    }

    std::cout << "\n--- Records in Perfect Hash Table with Magnitude >= " << std::fixed << std::setprecision(1) << min_mag_val << " ---" << std::endl;
    int found_count = 0;
    int current_overall_count = 0; // for separator line logic

    for (const auto& sec_hash_table_info : primary_table) {
        for (const auto& node : sec_hash_table_info.slots) {
            if (node.is_valid) {
                current_overall_count++;
                double current_rec_mag;
                if (KDTreeHelpers::try_string_to_double(node.record.magnitude, current_rec_mag, "", true)) {
                    if (current_rec_mag >= min_mag_val) {
                        found_count++;
                        std::cout << "Filtered Record #" << found_count << " (Key: "" << node.key << ""):" << std::endl;
                        std::cout << "  "; displayRecord(node.record);
                        if (found_count > 0 && static_cast<size_t>(current_overall_count) < num_elements) {
                             std::cout << "  --------------------------------------------------------------------------------" << std::endl;
                        }
                    }
                }
            }
        }
    }
    if (found_count == 0) {
        std::cout << "Nenhum registro encontrado com magnitude >= " << min_mag_str << "." << std::endl;
    }
     if (found_count > 0) {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}

void PerfectHashTable::list_by_year(const std::string& year_str) const {
     if (is_empty()) {
        std::cout << "Perfect Hash Table is empty. Nothing to list." << std::endl;
        return;
    }
    if (year_str.length() != 4) {
        std::cerr << "Erro: Formato de ano invalido. Use AAAA (ex: 2023)." << std::endl;
        return;
    }
    try { std::stoi(year_str); } 
    catch (const std::exception&) {
        std::cerr << "Erro: Ano invalido: "" << year_str << """ << std::endl;
        return;
    }

    std::cout << "\n--- Records in Perfect Hash Table do Ano " << year_str << " ---" << std::endl;
    int found_count = 0;
    int current_overall_count = 0;

    for (const auto& sec_hash_table_info : primary_table) {
        for (const auto& node : sec_hash_table_info.slots) {
            if (node.is_valid) {
                current_overall_count++;
                if (node.record.date.rfind(year_str, 0) == 0) { // Check if date string starts with year_str
                    found_count++;
                    std::cout << "Filtered Record #" << found_count << " (Key: "" << node.key << ""):" << std::endl;
                    std::cout << "  "; displayRecord(node.record);
                    if (found_count > 0 && static_cast<size_t>(current_overall_count) < num_elements) {
                        std::cout << "  --------------------------------------------------------------------------------" << std::endl;
                    }
                }
            }
        }
    }
    if (found_count == 0) {
        std::cout << "Nenhum registro encontrado para o ano " << year_str << "." << std::endl;
    }
    if (found_count > 0) {
        std::cout << "--------------------------------------------------------------------------------" << std::endl;
    }
}