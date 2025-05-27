#include "crow_all.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>

// Node for Huffman tree
struct Node {
    unsigned char ch;
    int freq;
    Node* left;
    Node* right;

    Node(unsigned char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

Node* buildTree(const std::unordered_map<unsigned char, int>& freqMap) {
    std::priority_queue<Node*, std::vector<Node*>, Compare> heap;

    for (auto& p : freqMap)
        heap.push(new Node(p.first, p.second));

    while (heap.size() > 1) {
        Node* left = heap.top(); heap.pop();
        Node* right = heap.top(); heap.pop();

        Node* merged = new Node('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        heap.push(merged);
    }

    return heap.top();
}

void generateCodes(Node* node, const std::string& code, std::unordered_map<unsigned char, std::string>& codes) {
    if (!node) return;
    if (!node->left && !node->right) codes[node->ch] = code;
    generateCodes(node->left, code + "0", codes);
    generateCodes(node->right, code + "1", codes);
}

std::vector<unsigned char> packBits(const std::vector<bool>& bits) {
    std::vector<unsigned char> bytes;
    int count = 0;
    unsigned char current = 0;
    for (bool b : bits) {
        current = (current << 1) | b;
        count++;
        if (count == 8) {
            bytes.push_back(current);
            count = 0;
            current = 0;
        }
    }
    if (count > 0) {
        current <<= (8 - count);
        bytes.push_back(current);
    }
    return bytes;
}

std::vector<unsigned char> compress(const std::vector<unsigned char>& input) {
    std::unordered_map<unsigned char, int> freq;
    for (auto c : input) freq[c]++;

    Node* root = buildTree(freq);
    std::unordered_map<unsigned char, std::string> codes;
    generateCodes(root, "", codes);

    std::vector<bool> bits;
    for (auto c : input) {
        for (char b : codes[c])
            bits.push_back(b == '1');
    }

    return packBits(bits);
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/compress").methods("POST"_method)([](const crow::request& req, crow::response& res) {
        auto file = req.get_file("file");
        if (file.empty()) {
            res.code = 400;
            res.end("No file uploaded");
            return;
        }

        std::ifstream in(file.path, std::ios::binary);
        std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        std::vector<unsigned char> compressed = compress(data);

        res.set_header("Content-Type", "application/octet-stream");
        res.set_header("Content-Disposition", "attachment; filename=\"compressed.huff\"");
        res.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
        res.end();
    });

    app.port(18080).multithreaded().run();
}
