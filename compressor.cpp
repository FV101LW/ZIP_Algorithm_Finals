#include "crow_all.h"
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <sstream>

using namespace std;

// Huffman tree node
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

Node* buildTree(const unordered_map<unsigned char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto& pair : freqMap)
        pq.push(new Node(pair.first, pair.second));

    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        Node* merged = new Node(0, left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        pq.push(merged);
    }
    return pq.top();
}

void generateCodes(Node* node, string code, unordered_map<unsigned char, string>& codes) {
    if (!node) return;
    if (!node->left && !node->right)
        codes[node->ch] = code;
    generateCodes(node->left, code + "0", codes);
    generateCodes(node->right, code + "1", codes);
}

void deleteTree(Node* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

vector<unsigned char> compress(const vector<unsigned char>& input) {
    unordered_map<unsigned char, int> freq;
    for (auto c : input) freq[c]++;

    Node* root = buildTree(freq);
    unordered_map<unsigned char, string> codes;
    generateCodes(root, "", codes);

    vector<bool> bits;
    for (auto c : input) {
        for (char bit : codes[c])
            bits.push_back(bit == '1');
    }

    vector<unsigned char> bytes;
    unsigned char current = 0;
    int count = 0;
    for (bool bit : bits) {
        current = (current << 1) | bit;
        if (++count == 8) {
            bytes.push_back(current);
            current = 0;
            count = 0;
        }
    }
    if (count > 0) {
        current <<= (8 - count);
        bytes.push_back(current);
    }

    deleteTree(root);
    return bytes;
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/compress").methods("POST"_method)([](const crow::request& req, crow::response& res) {
        const auto& contentType = req.get_header_value("Content-Type");
        if (contentType.find("multipart/form-data") == string::npos) {
            res.code = 400;
            res.write("Invalid content type");
            res.end();
            return;
        }

        auto files = crow::multipart::form(req);
        if (files.files.empty()) {
            res.code = 400;
            res.write("No file uploaded");
            res.end();
            return;
        }

        const auto& file = files.files[0];
        const string& fileData = file.body;

        vector<unsigned char> input(fileData.begin(), fileData.end());
        vector<unsigned char> compressed = compress(input);

        res.set_header("Content-Type", "application/octet-stream");
        res.set_header("Content-Disposition", "attachment; filename=\"compressed.huff\"");
        res.body.assign((const char*)compressed.data(), compressed.size());
        res.end();
    });

    app.port(18080).multithreaded().run();
}
