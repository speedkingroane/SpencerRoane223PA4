#include "REPL.h"
#include "CSV.h"
#include "Index.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>

static void printHelp() {
    std::cout <<
        "\nAvailable commands:\n"
        "  PEEK filename.csv\n"
        "      Show column indices, names, and a sample value from row 2.\n"
        "      Does not load data or build any index.\n\n"
        "  UPLOAD filename.csv PRIMARY_COL\n"
        "      Parse the full CSV, build a B+ tree index keyed on PRIMARY_COL,\n"
        "      and save it to <filename>_pi.db.\n\n"
        "  FIND csv_name key_value\n"
        "      Load <csv_name>_pi.db and return the record matching key_value.\n\n"
        "  HELP\n"
        "      Print this message.\n\n"
        "  EXIT\n"
        "      Quit the engine.\n\n";
}

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

void REPL::run() {
    std::cout << "B+ Tree Query Engine  (type HELP for commands)\n\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;   // EOF

        line = trim(line);
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        cmd = toUpper(cmd);

        try {
            if (cmd == "PEEK") {
                std::string filename;
                iss >> filename;
                if (filename.empty()) {
                    std::cout << "Usage: PEEK filename.csv\n";
                    continue;
                }
                CSV::peek(filename);

            } else if (cmd == "UPLOAD") {
                std::string filename, primaryCol;
                iss >> filename;
                std::getline(iss, primaryCol);
                primaryCol = trim(primaryCol);
                if (filename.empty() || primaryCol.empty()) {
                    std::cout << "Usage: UPLOAD filename.csv PRIMARY_COL\n";
                    continue;
                }
                Index::build(filename, primaryCol);

            } else if (cmd == "FIND") {
                std::string csvName, key;
                iss >> csvName;
                std::getline(iss, key);
                key = trim(key);
                if (csvName.empty() || key.empty()) {
                    std::cout << "Usage: FIND csv_name key_value\n";
                    continue;
                }
                Index::find(csvName, key);

            } else if (cmd == "HELP") {
                printHelp();

            } else if (cmd == "EXIT" || cmd == "QUIT") {
                std::cout << "Goodbye.\n";
                break;

            } else {
                std::cout << "Unknown command: \"" << cmd
                          << "\". Type HELP for available commands.\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}
