#ifndef PERFECT_HASH_H
#define PERFECT_HASH_H

#include <string>
#include <vector>
#include <list>       // For initial grouping, though secondary tables will be vectors
#include <functional> // For std::hash
#include <stdexcept>  // For std::runtime_error
#include "csv_reader.h" // For EarthquakeRecord and displayRecord (assumed)

// Forward declaration if KDTreeHelpers is in its own header
namespace KDTreeHelpers {
    bool try_string_to_double(const std::string& s, double& out, const std::string& field_name_for_error = "", bool suppress_error_message = false);
}


// Node structure, same as your original HashTable
struct HashNodePerfect {
    std::string key;
    EarthquakeRecord record;
    bool is_valid; // To handle deletions without actual removal from vector storage easily

    HashNodePerfect(std::string k, const EarthquakeRecord& r) : key(std::move(k)), record(r), is_valid(true) {}
    HashNodePerfect() : is_valid(false) {} // Default for empty slots
};

// Structure for secondary hash tables
struct SecondaryHashTable {
    size_t size;        // Size of this secondary table (e.g., n_i^2)
    size_t seed;        // Seed for the collision-free hash function for this bucket
    std::vector<HashNodePerfect> slots; // The actual secondary hash table slots

    SecondaryHashTable() : size(0), seed(1) {}
};

class PerfectHashTable {
private:
    std::vector<SecondaryHashTable> primary_table;
    size_t num_elements;
    size_t primary_table_target_size; // Desired size for the primary table (e.g., N)

    // Primary hash function
    size_t h_primary(const std::string& key) const;

    // Secondary hash function
    size_t h_secondary(const std::string& key, size_t seed_val, size_t mod_val) const;

    // Key generation (same as your original)
    std::string generate_key(const EarthquakeRecord& record) const;
    std::string generate_key(const std::string& date, const std::string& time_utc, const std::string& city) const;


public:
    // Constructor can take a hint for the number of expected elements
    explicit PerfectHashTable(size_t expected_elements = 100);

    // --- Core Perfect Hashing Method ---
    // Builds the two-level perfect hash table from a set of records
    void build_table(const std::vector<EarthquakeRecord>& records);

    // --- Standard Operations (adapted) ---
    // Insert might be tricky post-build. For simplicity, this might break "perfectness"
    // or require local re-hashing. This version will be basic.
    void insert_record(const EarthquakeRecord& record);

    const EarthquakeRecord* search_record(const std::string& date, const std::string& time_utc, const std::string& city) const;
    
    // Remove will mark the node as invalid.
    bool remove_record(const std::string& date, const std::string& time_utc, const std::string& city);

    int get_count() const;
    bool is_empty() const;

    // --- Utility and Listing (similar to your HashTable) ---
    void display_table_structure() const; // Shows primary and secondary structure
    std::vector<EarthquakeRecord> get_all_records_vector() const;
    void list_all_records() const;
    void list_by_min_magnitude(const std::string& min_mag_str) const;
    void list_by_year(const std::string& year_str) const;
};

#endif // PERFECT_HASH_H