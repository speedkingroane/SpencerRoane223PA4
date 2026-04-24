#include "CSV.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>

std::vector<std::string> CSV::parseLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (inQuotes) {
            if (c == '"') {
                // Doubled quote inside quoted field → literal quote
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field += c;
            }
        } else {
            if (c == '"') {
                inQuotes = true;
            } else if (c == ',') {
                fields.push_back(field);
                field.clear();
            } else if (c == '\r') {
                // skip Windows carriage return
            } else {
                field += c;
            }
        }
    }
    fields.push_back(field);
    return fields;
}

void CSV::peek(const std::string& filename) {
    std::ifstream f(filename);
    if (!f) throw std::runtime_error("Cannot open file: " + filename);

    std::string headerLine, dataLine;
    if (!std::getline(f, headerLine))
        throw std::runtime_error("File is empty: " + filename);
    if (!std::getline(f, dataLine))
        throw std::runtime_error("File has only a header row: " + filename);

    auto headers = parseLine(headerLine);
    auto values  = parseLine(dataLine);

    for (size_t i = 0; i < headers.size(); i++) {
        std::string val = (i < values.size()) ? values[i] : "";
        std::cout << "Col " << std::setw(2) << std::left << i
                  << "  " << std::setw(24) << std::left << headers[i]
                  << "  " << val << "\n";
    }
}
