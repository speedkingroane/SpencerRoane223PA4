#pragma once
#include <string>
#include <vector>
#include <utility>

class BPTree {
public:
    struct Node {
        bool isLeaf;
        std::vector<std::string> keys;
        std::vector<std::string> values;   // leaf nodes only
        std::vector<Node*> children;       // internal nodes only
        Node* next;                        // leaf linked list

        explicit Node(bool leaf) : isLeaf(leaf), next(nullptr) {}
    };

    explicit BPTree(int order = 128);
    ~BPTree();

    // Returns false if key already exists (duplicate)
    bool insert(const std::string& key, const std::string& value);
    std::string* search(const std::string& key);
    void printTree() const;

    void save(const std::string& path, const std::vector<std::string>& headers) const;
    static BPTree* load(const std::string& path, std::vector<std::string>& headers);

private:
    Node* root;
    int order;

    std::pair<std::string, Node*> insertRec(Node* node, const std::string& key,
                                             const std::string& value, bool& duplicate);
    void destroyTree(Node* node);
    void printNode(Node* node, int depth) const;
};
