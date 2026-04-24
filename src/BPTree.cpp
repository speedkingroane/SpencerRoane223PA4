#include "BPTree.h"
#include <algorithm>
#include <iostream>

BPTree::BPTree() : root(nullptr) {}

BPTree::~BPTree() {
    destroyTree(root);
}

void BPTree::destroyTree(Node* node) {
    if (!node) return;
    if (!node->isLeaf)
        for (Node* child : node->children)
            destroyTree(child);
    delete node;
}

void BPTree::insert(const std::string& key, const std::string& value) {
    if (!root) {
        root = new Node(true);
        root->keys.push_back(key);
        root->values.push_back(value);
        return;
    }

    auto [promKey, newNode] = insertRec(root, key, value);

    if (newNode) {
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(promKey);
        newRoot->children.push_back(root);
        newRoot->children.push_back(newNode);
        root = newRoot;
    }
}

std::pair<std::string, BPTree::Node*> BPTree::insertRec(Node* node, const std::string& key, const std::string& value) {
    if (node->isLeaf) {
        auto pos = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        int idx = pos - node->keys.begin();
        node->keys.insert(pos, key);
        node->values.insert(node->values.begin() + idx, value);

        if ((int)node->keys.size() < ORDER)
            return {"", nullptr};

        // Leaf split: copy mid key up, right node keeps it
        int mid = ORDER / 2;
        Node* right = new Node(true);
        right->keys.assign(node->keys.begin() + mid, node->keys.end());
        right->values.assign(node->values.begin() + mid, node->values.end());
        node->keys.resize(mid);
        node->values.resize(mid);
        right->next = node->next;
        node->next = right;
        return {right->keys[0], right};
    }

    // Internal node: route to correct child
    int childIdx = std::upper_bound(node->keys.begin(), node->keys.end(), key) - node->keys.begin();
    auto [promKey, newChild] = insertRec(node->children[childIdx], key, value);

    if (!newChild)
        return {"", nullptr};

    int keyIdx = std::lower_bound(node->keys.begin(), node->keys.end(), promKey) - node->keys.begin();
    node->keys.insert(node->keys.begin() + keyIdx, promKey);
    node->children.insert(node->children.begin() + keyIdx + 1, newChild);

    if ((int)node->keys.size() < ORDER)
        return {"", nullptr};

    // Internal split: mid key moves up (not copied), removed from both halves
    int mid = ORDER / 2;
    std::string pushUp = node->keys[mid];
    Node* right = new Node(false);
    right->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
    right->children.assign(node->children.begin() + mid + 1, node->children.end());
    node->keys.resize(mid);
    node->children.resize(mid + 1);
    return {pushUp, right};
}

std::string* BPTree::search(const std::string& key) {
    if (!root) return nullptr;

    Node* curr = root;
    while (!curr->isLeaf) {
        int idx = std::upper_bound(curr->keys.begin(), curr->keys.end(), key) - curr->keys.begin();
        curr = curr->children[idx];
    }

    auto pos = std::lower_bound(curr->keys.begin(), curr->keys.end(), key);
    if (pos != curr->keys.end() && *pos == key)
        return &curr->values[pos - curr->keys.begin()];

    return nullptr;
}

void BPTree::printTree() const {
    printNode(root, 0);
    std::cout << "\nLeaf chain: ";
    Node* leaf = root;
    if (leaf) {
        while (!leaf->isLeaf) leaf = leaf->children[0];
        while (leaf) {
            for (const auto& k : leaf->keys) std::cout << k << " ";
            std::cout << "| ";
            leaf = leaf->next;
        }
    }
    std::cout << "\n";
}

void BPTree::printNode(Node* node, int depth) const {
    if (!node) return;
    std::string indent(depth * 4, ' ');
    if (node->isLeaf) {
        std::cout << indent << "[L] ";
        for (const auto& k : node->keys) std::cout << k << " ";
    } else {
        std::cout << indent << "[I] ";
        for (const auto& k : node->keys) std::cout << k << " ";
        std::cout << "\n";
        for (Node* child : node->children) printNode(child, depth + 1);
        return;
    }
    std::cout << "\n";
}
