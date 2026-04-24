#include "Index.h"
#include "BPTree.h"
#include "CSV.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>

// ── field packing: join parsed fields with ASCII unit-separator (0x1F) ────────
static const char SEP = '\x1F';

static std::string packFields(const std::vector<std::string>& fields) {
    std::string out;
    for (size_t i = 0; i < fields.size(); i++) {
        if (i) out += SEP;
        out += fields[i];
    }
    return out;
}

static std::vector<std::string> unpackFields(const std::string& packed) {
    std::vector<std::string> fields;
    std::string field;
    for (char c : packed) {
        if (c == SEP) { fields.push_back(field); field.clear(); }
        else           field += c;
    }
    fields.push_back(field);
    return fields;
}

// Strip directory path, remove extension
static std::string baseName(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    std::string name = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = name.rfind('.');
    return (dot != std::string::npos) ? name.substr(0, dot) : name;
}

// ── UPLOAD ────────────────────────────────────────────────────────────────────
void Index::build(const std::string& csvPath, const std::string& primaryCol) {
    std::ifstream f(csvPath);
    if (!f) throw std::runtime_error("Cannot open file: " + csvPath);

    std::string headerLine;
    if (!std::getline(f, headerLine))
        throw std::runtime_error("File is empty: " + csvPath);

    auto headers = CSV::parseLine(headerLine);

    int pkIdx = -1;
    for (int i = 0; i < (int)headers.size(); i++)
        if (headers[i] == primaryCol) { pkIdx = i; break; }
    if (pkIdx < 0)
        throw std::runtime_error("Column \"" + primaryCol + "\" not found in header");

    BPTree tree(128);
    std::string line;
    int rowNum = 1;
    int loaded = 0;

    while (std::getline(f, line)) {
        ++rowNum;
        if (line.empty() || line == "\r") continue;

        auto fields = CSV::parseLine(line);

        if ((int)fields.size() <= pkIdx) {
            std::cerr << "Warning: row " << rowNum
                      << " has too few fields, skipping\n";
            continue;
        }

        std::string key = fields[pkIdx];
        if (key.empty()) {
            std::cerr << "Warning: row " << rowNum
                      << " has empty primary key, skipping\n";
            continue;
        }

        if (!tree.insert(key, packFields(fields)))
            throw std::runtime_error(
                "Duplicate key \"" + key + "\" at row " + std::to_string(rowNum));

        ++loaded;
    }

    std::string outPath = baseName(csvPath) + "_pi.db";
    tree.save(outPath, headers);

    std::cout << "Indexed " << loaded << " records.\n";
    std::cout << "Index saved to: " << outPath << "\n";
}

// ── FIND ──────────────────────────────────────────────────────────────────────
void Index::find(const std::string& csvName, const std::string& key) {
    std::string dbPath = csvName + "_pi.db";

    std::vector<std::string> headers;
    BPTree* tree = BPTree::load(dbPath, headers);
    if (!tree)
        throw std::runtime_error("Index file not found: " + dbPath);

    std::string* ptr = tree->search(key);

    if (!ptr) {
        delete tree;
        std::cout << "Not found: \"" << key << "\"\n";
        return;
    }

    // Copy value before freeing the tree
    std::vector<std::string> fields = unpackFields(*ptr);
    delete tree;

    // Find longest header for alignment
    size_t maxLen = 0;
    for (const auto& h : headers)
        maxLen = std::max(maxLen, h.size());
    maxLen += 2;

    std::cout << "\n";
    for (size_t i = 0; i < headers.size(); i++) {
        std::string val = (i < fields.size()) ? fields[i] : "";
        std::cout << "  " << std::left << std::setw((int)maxLen) << headers[i]
                  << ": " << val << "\n";
    }
    std::cout << "\n";
}
