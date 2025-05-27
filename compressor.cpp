#include "crow_all.h"
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <fstream>

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

// Build Huffman Tree from frequency map
Node* buildHuffmanTree(const unordered_map<unsigned char, int>& freqMap) {
    priority_queue<Node*, vector<Node*>, Compare> minHeap;

    for (const auto& pair : freqMap) {
        minHeap.push(new Node(pair.first, pair.second));
    }

    while (minHeap.size() > 1) {
        Node* left = minHeap.top(); minHeap.pop();
        Node* right = minHeap.top(); minHeap.pop();

        Node* internalNode = new Node('\0', left->freq + right->freq);
        internalNode->left = left;
        internalNode->right = right;

        minHeap.push(internalNode);
    }
    return minHeap.top();
}

// Recursively generate Huffman codes
void generateCodes(Node* root, const string& code, unordered_map<unsigned char, string>& huffmanCodes) {
    if (!root) return;

    if (!root->left && !root->right) {
        huffmanCodes[root->ch] = code;
    }

    generateCodes(root->left, code + "0", huffmanCodes);
    generateCodes(root->right, code + "1", huffmanCodes);
}

// Delete Huffman tree nodes
void deleteTree(Node* root) {
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

// Pack vector<bool> bits into vector<unsigned char> bytes
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
    // If bits remain not full byte, pad with 0s on the right
    if (bitCount > 0) {
        currentByte <<= (8 - bitCount);
        bytes.push_back(currentByte);
    }
    return bytes;
}

// Compress binary data with Huffman coding
vector<unsigned char> compress(const vector<unsigned char>& input) {
    // Build frequency map
    unordered_map<unsigned char, int> freqMap;
    for (unsigned char ch : input) {
        freqMap[ch]++;
    }

    // Build Huffman tree
    Node* root = buildHuffmanTree(freqMap);

    // Generate codes
    unordered_map<unsigned char, string> huffmanCodes;
    generateCodes(root, "", huffmanCodes);

    // Encode input
    vector<bool> compressedBits;
    for (unsigned char ch : input) {
        string code = huffmanCodes[ch];
        for (char bit : code) {
            compressedBits.push_back(bit == '1');
        }
    }

    // Pack bits into bytes
    vector<unsigned char> compressedBytes = packBits(compressedBits);

    deleteTree(root);

    // NOTE: For real decompression, you also need to store the tree or codes as header.
    // Here, for demo, we only return compressed data.

    return compressedBytes;
}

// Read entire file as binary vector
vector<unsigned char> readBinary(const string& filename) {
    ifstream file(filename, ios::binary);
    return vector<unsigned char>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

// Crow handler for file upload and compression
void handleUpload(const crow::request& req, crow::response& res) {
    auto files = req.get_file("files[]");
    if (files.empty()) {
        res.code = 400;
        res.write("No file uploaded");
        res.end();
        return;
    }

    // For simplicity, compress the first uploaded file
    auto& file = files[0];

    // Read uploaded file content into vector<unsigned char>
    ifstream inputFile(file.path, ios::binary);
    vector<unsigned char> buffer((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());

    // Compress the data
    vector<unsigned char> compressedData = compress(buffer);

    // Set response headers to trigger file download
    res.set_header("Content-Type", "application/octet-stream");
    res.set_header("Content-Disposition", "attachment; filename=\"compressed.huff\"");

    // Write compressed binary data to response
    res.write((const char*)compressedData.data(), compressedData.size());
    res.end();
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/compress").methods("POST"_method)([](const crow::request& req, crow::response& res) {
        handleUpload(req, res);
    });

    app.port(18080).multithreaded().run();
}
