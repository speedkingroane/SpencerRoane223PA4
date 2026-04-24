#pragma once
#include <string>

class Index {
public:
    // UPLOAD: parse csvPath, build B+ tree keyed on primaryCol, write <basename>_pi.db
    static void build(const std::string& csvPath, const std::string& primaryCol);

    // FIND: load csvName_pi.db, search for key, print full record
    static void find(const std::string& csvName, const std::string& key);
};
