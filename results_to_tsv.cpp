/*******************************************************************************
 * results_to_tsv.cpp
 *
 * Convert RESULT lines into a tab-separated values file (.tsv)
 *
 * Copyright (C) 2020 Timo Bingmann <tb@panthema.net>
 *
 * All rights reserved. Published under the MIT License in the LICENSE file.
 ******************************************************************************/

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

/******************************************************************************/
// string methods borrowed from tlx

//! Checks if the given match string is located at the start of this string.
bool starts_with(const std::string& str, const char* match) {
    std::string::const_iterator s = str.begin();
    while (*match != 0) {
        if (s == str.end() || *s != *match)
            return false;
        ++s, ++match;
    }
    return true;
}

//! Split the given string at each separator character into distinct substrings.
std::vector<std::string>& split(std::vector<std::string>* into, char sep,
    const std::string& str, std::string::size_type limit = std::string::npos) {

    into->clear();
    if (limit == 0)
        return *into;

    std::string::const_iterator it = str.begin(), last = it;

    for (; it != str.end(); ++it) {
        if (*it == sep) {
            if (into->size() + 1 >= limit) {
                into->emplace_back(last, str.end());
                return *into;
            }

            into->emplace_back(last, it);
            last = it + 1;
        }
    }

    into->emplace_back(last, it);

    return *into;
}

//! Simple, fast, but "insecure" string hash method by sdbm database.
uint32_t hash_sdbm(const unsigned char* str, size_t size) {
    uint32_t hash = 0;
    while (size-- > 0) {
        hash = static_cast<unsigned char>(*str++) + (hash << 6) + (hash << 16) -
               hash;
    }
    return hash;
}

//! Simple, fast, but "insecure" string hash method by sdbm database.
uint32_t hash_sdbm(const char* str, size_t size) {
    return hash_sdbm(reinterpret_cast<const unsigned char*>(str), size);
}

//! Simple, fast, but "insecure" string hash method by sdbm database.
uint32_t hash_sdbm(const std::string& str) {
    return hash_sdbm(str.data(), str.size());
}

/******************************************************************************/

//! column headers of result TSV file
std::vector<std::string> s_keys;
//! hashes of keys for faster lookup
std::vector<uint32_t> s_keys_hash;

//! lookup string in key index, create new column if it does not exist.
size_t lookup_key(const std::string& str) {
    uint32_t hash = hash_sdbm(str);
    for (size_t i = 0; i < s_keys.size(); ++i) {
        if (s_keys_hash[i] != hash || s_keys[i] != str)
            continue;
        return i;
    }
    s_keys.push_back(str);
    s_keys_hash.push_back(hash);
    return s_keys.size() - 1;
}

//! rows are stripped of keys and values are ordered as in s_keys.
struct Row {
    std::vector<std::string> fields;
};

//! TSV rows collected
std::vector<Row> s_rows;

//! process each file or input stream
void process_stream(std::istream& is) {
    std::string line;
    std::vector<std::string> fields;
    while (std::getline(is, line)) {
        if (!starts_with(line, "RESULT\t"))
            continue;
        split(&fields, '\t', line);

        Row row;
        row.fields.resize(s_keys.size());

        for (size_t i = 1; i < fields.size(); ++i) {
            std::string::size_type eq_pos = fields[i].find('=');
            if (eq_pos == std::string::npos)
                continue;

            std::string key = fields[i].substr(0, eq_pos);
            std::string val = fields[i].substr(eq_pos + 1);

            size_t index = lookup_key(key);
            if (row.fields.size() < index + 1)
                row.fields.resize(index + 1);
            row.fields[index] = val;
        }

        s_rows.push_back(row);
    }
}

//! output TSV
void output_tsv() {
    for (size_t i = 0; i < s_keys.size(); ++i) {
        if (i != 0)
            std::cout << '\t';
        std::cout << s_keys[i];
    }
    std::cout << '\n';

    for (size_t r = 0; r < s_rows.size(); ++r) {
        for (size_t i = 0; i < s_keys.size(); ++i) {
            if (i != 0)
                std::cout << '\t';

            if (i < s_rows[r].fields.size())
                std::cout << s_rows[r].fields[i];
        }
        std::cout << '\n';
    }
}

//! main
int main(int argc, char* argv[]) {
    if (argc == 1) {
        std::cerr << "Reading stdin." << std::endl;
        process_stream(std::cin);
    }
    else {
        for (int i = 1; i < argc; ++i) {
            std::ifstream is(argv[i]);
            if (!is.good()) {
                std::cerr << "Error opening \"" << argv[i]
                          << "\": " << strerror(errno) << std::endl;
            }
            else {
                std::cerr << "Reading \"" << argv[i] << "\"." << std::endl;
                process_stream(is);
            }
        }
    }

    std::cerr << "Read " << s_rows.size() << " rows containing "
              << s_keys.size() << " columns." << std::endl;

    output_tsv();

    return 0;
}

/******************************************************************************/
