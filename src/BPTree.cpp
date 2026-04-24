#include "BPTree.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <stdexcept>

// ── binary I/O helpers ────────────────────────────────────────────────────────
static void writeU32(std::ofstream& out, uint32_t v) {
    out.write(reinterpret_cast<const char*>(&v), 4);
}
static void writeI32(std::ofstream& out, int32_t v) {
    out.write(reinterpret_cast<const char*>(&v), 4);
}
static void writeStr(std::ofstream& out, const std::string& s) {
    writeU32(out, static_cast<uint32_t>(s.size()));
    out.write(s.data(), s.size());
}
static uint32_t readU32(std::ifstream& in) {
    uint32_t v; in.read(reinterpret_cast<char*>(&v), 4); return v;
}
static int32_t readI32(std::ifstream& in) {
    int32_t v; in.read(reinterpret_cast<char*>(&v), 4); return v;
}
static std::string readStr(std::ifstream& in) {
    uint32_t len = readU32(in);
    std::string s(len, '\0');
    in.read(&s[0], len);
    return s;
}

// ── collect nodes via pre-order DFS ──────────────────────────────────────────
static void collectNodes(BPTree::Node* node,
                         std::vector<BPTree::Node*>& nodes,
                         std::unordered_map<BPTree::Node*, int>& idx) {
    if (!node) return;
    idx[node] = static_cast<int>(nodes.size());
    nodes.push_back(node);
    if (!node->isLeaf)
        for (BPTree::Node* child : node->children)
            collectNodes(child, nodes, idx);
}

// ── BPTree ────────────────────────────────────────────────────────────────────
BPTree::BPTree(int order) : root(nullptr), order(order) {}

BPTree::~BPTree() { destroyTree(root); }

void BPTree::destroyTree(Node* node) {
    if (!node) return;
    if (!node->isLeaf)
        for (Node* child : node->children)
            destroyTree(child);
    delete node;
}

bool BPTree::insert(const std::string& key, const std::string& value) {
    if (!root) {
        root = new Node(true);
        root->keys.push_back(key);
        root->values.push_back(value);
        return true;
    }

    bool duplicate = false;
    auto [promKey, newNode] = insertRec(root, key, value, duplicate);
    if (duplicate) return false;

    if (newNode) {
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(promKey);
        newRoot->children.push_back(root);
        newRoot->children.push_back(newNode);
        root = newRoot;
    }
    return true;
}

std::pair<std::string, BPTree::Node*>
BPTree::insertRec(Node* node, const std::string& key,
                  const std::string& value, bool& duplicate) {
    if (node->isLeaf) {
        auto pos = std::lower_bound(node->keys.begin(), node->keys.end(), key);
        if (pos != node->keys.end() && *pos == key) {
            duplicate = true;
            return {"", nullptr};
        }
        int idx = static_cast<int>(pos - node->keys.begin());
        node->keys.insert(pos, key);
        node->values.insert(node->values.begin() + idx, value);

        if (static_cast<int>(node->keys.size()) < order)
            return {"", nullptr};

        // Leaf split — mid key copied up, stays in right node
        int mid = order / 2;
        Node* right = new Node(true);
        right->keys.assign(node->keys.begin() + mid, node->keys.end());
        right->values.assign(node->values.begin() + mid, node->values.end());
        node->keys.resize(mid);
        node->values.resize(mid);
        right->next = node->next;
        node->next = right;
        return {right->keys[0], right};
    }

    // Internal node — route to correct child
    int childIdx = static_cast<int>(
        std::upper_bound(node->keys.begin(), node->keys.end(), key)
        - node->keys.begin());
    auto [promKey, newChild] = insertRec(node->children[childIdx], key, value, duplicate);
    if (!newChild) return {"", nullptr};

    int keyIdx = static_cast<int>(
        std::lower_bound(node->keys.begin(), node->keys.end(), promKey)
        - node->keys.begin());
    node->keys.insert(node->keys.begin() + keyIdx, promKey);
    node->children.insert(node->children.begin() + keyIdx + 1, newChild);

    if (static_cast<int>(node->keys.size()) < order)
        return {"", nullptr};

    // Internal split — mid key moves up, removed from both halves
    int mid = order / 2;
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
        int idx = static_cast<int>(
            std::upper_bound(curr->keys.begin(), curr->keys.end(), key)
            - curr->keys.begin());
        curr = curr->children[idx];
    }
    auto pos = std::lower_bound(curr->keys.begin(), curr->keys.end(), key);
    if (pos != curr->keys.end() && *pos == key)
        return &curr->values[pos - curr->keys.begin()];
    return nullptr;
}

// ── serialization ─────────────────────────────────────────────────────────────
void BPTree::save(const std::string& path,
                  const std::vector<std::string>& headers) const {
    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot write index file: " + path);

    std::vector<Node*> nodes;
    std::unordered_map<Node*, int> nodeIdx;
    collectNodes(root, nodes, nodeIdx);

    out.write("BPDB", 4);
    writeU32(out, static_cast<uint32_t>(order));

    writeU32(out, static_cast<uint32_t>(headers.size()));
    for (const auto& h : headers) writeStr(out, h);

    writeU32(out, static_cast<uint32_t>(nodes.size()));
    writeI32(out, root ? nodeIdx[root] : -1);

    for (Node* n : nodes) {
        uint8_t leaf = n->isLeaf ? 1 : 0;
        out.write(reinterpret_cast<const char*>(&leaf), 1);
        writeU32(out, static_cast<uint32_t>(n->keys.size()));
        for (const auto& k : n->keys) writeStr(out, k);

        if (n->isLeaf) {
            for (const auto& v : n->values) writeStr(out, v);
            writeI32(out, n->next ? nodeIdx.at(n->next) : -1);
        } else {
            for (Node* child : n->children)
                writeU32(out, static_cast<uint32_t>(nodeIdx.at(child)));
        }
    }
}

// ── deserialization ───────────────────────────────────────────────────────────
BPTree* BPTree::load(const std::string& path, std::vector<std::string>& headers) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return nullptr;

    char magic[4];
    in.read(magic, 4);
    if (std::memcmp(magic, "BPDB", 4) != 0)
        throw std::runtime_error("Invalid index file: " + path);

    int order = static_cast<int>(readU32(in));
    BPTree* tree = new BPTree(order);

    uint32_t headerCount = readU32(in);
    headers.resize(headerCount);
    for (auto& h : headers) h = readStr(in);

    uint32_t nodeCount = readU32(in);
    int32_t rootIdx    = readI32(in);
    if (nodeCount == 0) return tree;

    std::vector<Node*> nodes(nodeCount);
    std::vector<std::vector<uint32_t>> childIdx(nodeCount);
    std::vector<int32_t> nextIdx(nodeCount, -1);

    for (uint32_t i = 0; i < nodeCount; i++) {
        uint8_t isLeaf;
        in.read(reinterpret_cast<char*>(&isLeaf), 1);
        nodes[i] = new Node(isLeaf != 0);

        uint32_t keyCount = readU32(in);
        nodes[i]->keys.resize(keyCount);
        for (auto& k : nodes[i]->keys) k = readStr(in);

        if (isLeaf) {
            nodes[i]->values.resize(keyCount);
            for (auto& v : nodes[i]->values) v = readStr(in);
            nextIdx[i] = readI32(in);
        } else {
            uint32_t childCount = keyCount + 1;
            childIdx[i].resize(childCount);
            for (auto& c : childIdx[i]) c = readU32(in);
        }
    }

    // Fix up pointers
    for (uint32_t i = 0; i < nodeCount; i++) {
        if (!nodes[i]->isLeaf)
            for (uint32_t ci : childIdx[i])
                nodes[i]->children.push_back(nodes[ci]);
        else if (nextIdx[i] >= 0)
            nodes[i]->next = nodes[nextIdx[i]];
    }

    tree->root = nodes[rootIdx];
    return tree;
}

// ── debug print ───────────────────────────────────────────────────────────────
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
    std::cout << indent << (node->isLeaf ? "[L] " : "[I] ");
    for (const auto& k : node->keys) std::cout << k << " ";
    std::cout << "\n";
    if (!node->isLeaf)
        for (Node* child : node->children) printNode(child, depth + 1);
}
