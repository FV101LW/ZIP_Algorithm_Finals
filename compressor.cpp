#include "crow_all.h"
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// Huffman Tree node
struct Node {
    unsigned char ch;
    int freq;
    Node* left;
    Node* right;

    Node(unsigned char character, int frequency) : ch(character), freq(frequency), left(nullptr), right(nullptr) {}
};

// Comparator for priority queue
struct Compare {
    bool operator()(Node* left, Node* right) {
        return left->freq > right->freq;
    }
};

Node* buildHuffmanTree(const unordered_map<unsigned char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, Compare> minHeap;
    for (const auto& pair : freqMap)
        minHeap.push(new Node(pair.first, pair.second));

    while (minHeap.size() > 1) {
        Node* left = minHeap.top(); minHeap.pop();
        Node* right = minHeap.top(); minHeap.pop();

        Node* internal = new Node('\0', left->freq + right->freq);
        internal->left = left;
        internal->right = right;
        minHeap.push(internal);
    }

    return minHeap.top();
}

void generateCodes(Node* root, const string& code, unordered_map<unsigned char, string>& huffmanCodes) {
    if (!root) return;
    if (!root->left && !root->right)
        huffmanCodes[root->ch] = code;
    generateCodes(root->left, code + "0", huffmanCodes);
    generateCodes(root->right, code + "1", huffmanCodes);
}

void deleteTree(Node* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

vector<unsigned char> packBits(const vector<bool>& bits) {
    vector<unsigned char> bytes;
    int bitCount = 0;
    unsigned char currentByte = 0;
    for (bool bit : bits) {
        currentByte = (currentByte << 1) | bit;
        bitCount++;
        if (bitCount == 8) {
            bytes.push_back(currentByte);
            bitCount = 0;
            currentByte = 0;
        }
    }
    if (bitCount > 0) {
        currentByte <<= (8 - bitCount);
        bytes.push_back(currentByte);
    }
    return bytes;
}

vector<unsigned char> compress(const vector<unsigned char>& input) {
    unordered_map<unsigned char, int> freqMap;
    for (unsigned char ch : input)
        freqMap[ch]++;

    Node* root = buildHuffmanTree(freqMap);

    unordered_map<unsigned char, string> huffmanCodes;
    generateCodes(root, "", huffmanCodes);

    vector<bool> compressedBits;
    for (unsigned char ch : input) {
        const string& code = huffmanCodes[ch];
        for (char bit : code)
            compressedBits.push_back(bit == '1');
    }

    vector<unsigned char> result = packBits(compressedBits);
    deleteTree(root);
    return result;
}

void handleUpload(const crow::request& req, crow::response& res) {
    auto files = req.get_files();
    if (files.find("files[]") == files.end()) {
        res.code = 400;
        res.write("No file uploaded");
        res.end();
        return;
    }

    const crow::multipart::part& filePart = files["files[]"][0];
    const string& fileContent = filePart.body;

    vector<unsigned char> data(fileContent.begin(), fileContent.end());
    vector<unsigned char> compressed = compress(data);

    res.set_header("Content-Type", "application/octet-stream");
    res.set_header("Content-Disposition", "attachment; filename=\"compressed.huff\"");
    res.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    res.end();
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/compress").methods("POST"_method)([](const crow::request& req, crow::response& res) {
        handleUpload(req, res);
    });

    app.port(18080).multithreaded().run();
}
