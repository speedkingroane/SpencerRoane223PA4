#pragma once
#include <string>
#include <vector>
#include <utility>

class BPTree {
public:
    // Max keys per node. ORDER=4 forces splits at 3 keys for easy testing.
    // Increase to 128+ for production CSV loads.
    static const int ORDER = 4;

    struct Node {
        bool isLeaf;
        std::vector<std::string> keys;
        std::vector<std::string> values;   // leaf nodes only
        std::vector<Node*> children;       // internal nodes only
        Node* next;                        // leaf linked list

        explicit Node(bool leaf) : isLeaf(leaf), next(nullptr) {}
    };

    BPTree();
    ~BPTree();

    void insert(const std::string& key, const std::string& value);
    std::string* search(const std::string& key);
    void printTree() const;

private:
    Node* root;

    // Returns {promoted_key, new_right_node} on split, {"", nullptr} otherwise
    std::pair<std::string, Node*> insertRec(Node* node, const std::string& key, const std::string& value);
    void destroyTree(Node* node);
    void printNode(Node* node, int depth) const;
};
