#pragma once
#include <string>
#include <vector>

class CSV {
public:
    // Parse one RFC-4180 CSV line into fields (handles quoted fields, escaped quotes, \r)
    static std::vector<std::string> parseLine(const std::string& line);

    // PEEK command: print column index, header name, and sample value from row 2
    static void peek(const std::string& filename);
};
