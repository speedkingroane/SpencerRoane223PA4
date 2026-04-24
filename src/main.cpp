#include "BPTree.h"
#include <iostream>
#include <vector>
#include <string>

int main() {
    BPTree tree;

    // 12 hardcoded entries — small ORDER=4 forces several splits
    std::vector<std::pair<std::string, std::string>> entries = {
        {"10001", "New York, NY"},
        {"90210", "Beverly Hills, CA"},
        {"30301", "Atlanta, GA"},
        {"60601", "Chicago, IL"},
        {"77001", "Houston, TX"},
        {"02101", "Boston, MA"},
        {"98101", "Seattle, WA"},
        {"33101", "Miami, FL"},
        {"85001", "Phoenix, AZ"},
        {"19101", "Philadelphia, PA"},
        {"80201", "Denver, CO"},
        {"15201", "Pittsburgh, PA"},
    };

    std::cout << "=== Inserting " << entries.size() << " entries ===\n";
    for (auto& [k, v] : entries) {
        std::cout << "  INSERT " << k << "\n";
        tree.insert(k, v);
    }

    std::cout << "\n=== Tree Structure ===\n";
    tree.printTree();

    std::cout << "\n=== Search Tests ===\n";
    // All 12 inserted keys must be found
    for (auto& [k, v] : entries) {
        std::string* r = tree.search(k);
        if (r) std::cout << "[OK]    FOUND     " << k << " => " << *r << "\n";
        else   std::cout << "[FAIL]  NOT FOUND " << k << " (should exist)\n";
    }

    std::cout << "\n--- Keys that should NOT be found ---\n";
    for (const std::string& k : {"00000", "55555", "12345", "99999"}) {
        std::string* r = tree.search(k);
        if (!r) std::cout << "[OK]    NOT FOUND " << k << "\n";
        else    std::cout << "[FAIL]  FOUND     " << k << " (should not exist)\n";
    }

    return 0;
}
