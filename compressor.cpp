#include "crow_all.h"
#include <zip.h>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
#include <bitset>

using namespace std;

struct Node {
      char ch;
      int freq;
      Node* left;
      Node* right;
}

Node (char character, freq frequency) : ch(character), freq(frequency), left(nullptr), right(nullptr) {}

struct Compare() {
       bool operator() (Node* left, Node* right) {
         return left->freq > right->freq;
       }
};

Node* buildHuffmanTree(const unordered_map<char, int>& freqMap) {
  priority_queue<Node*, vector<Node*>, Compare> minHeap;

  for(const auto& pair : freqMap) {
    minHeap.push(new Node(pair.first, pair.second));
  }

  while(minHeap.size() > 1) {
    Node* left = minHeap.top(); minHeap.pop();
    Node* right = minHeap.top(); minHeap.pop();

    Node* internalNode = newNode('\0', left->freq + right->freq);
    internalNode->left = left;
    internalNode->right = right;

    minHeap.push(internalNode);
  }
  return minHeap.top();
}

void generateCodes(Node* root, const string& code, unordered_map<char, string>& huffmanCodes) {
  if(!root) return;

  if(root->left == nullptr && root->right == nullptr) {
    huffmanCodes[root->ch] = code;
  }

  generateCodes(root->left, code + "0", huffmanCodes);
  generateCodes(root->right, code + "1", huffmanCodes);
}

string compress(const string& input) {
  unordered_map<char, int> freqMap;

  for(char ch : input) {
    freqMap[ch]++;
  }

  Node* root = buildHuffmanTree(freqMap); // Build Huffman tree
  // Generate Huffman Algorithm
  unordered_map<char, string> huffmanCodes;

  generateCodes(root, "", huffmanCodes);

  string compressedOut;  // Create compressed output
  for(char ch : input) {
    compressedOut += huffmanCodes[ch];
  }
  return compressedOut;
}

//void deleteTree(Node* root) {
//}

int main() {
  crow::SimpleApp app;

  CROW_ROUTE(app, "/compress").methods("POST"_method)([](const crow::request& req) {
    
  }
}
