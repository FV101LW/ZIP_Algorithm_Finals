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
      unsigned char ch; // For binary data & files(Videos, images, audio)
      int freq;
      Node* left;
      Node* right;

    Node (unsigned char character, freq frequency) : ch(character), freq(frequency), left(nullptr), right(nullptr) {}
};



struct Compare() {
       bool operator() (Node* left, Node* right) {
         return left->freq > right->freq;
       }
};

Node* buildHuffmanTree(const unordered_map<unsigned char, int>& freqMap) {
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

void generateCodes(Node* root, const string& code, unordered_map<unsigned char, string>& huffmanCodes) {
  if(!root) return;

  if(root->left == nullptr && root->right == nullptr) {
    huffmanCodes[root->ch] = code;
  }

  generateCodes(root->left, code + "0", huffmanCodes);
  generateCodes(root->right, code + "1", huffmanCodes);
}

string compress(const string& input) {
  unordered_map<unsigned char, int> freqMap;

  for(unsigned char ch : input) {
    freqMap[ch]++;
  }

  Node* root = buildHuffmanTree(freqMap); // Build Huffman tree
  // Generate Huffman Algorithm
  unordered_map<char, string> huffmanCodes;

  generateCodes(root, "", huffmanCodes);

  vector<bool> compressedOut;  // Create compressed output
  for(unsigned char ch : input) {
        string code = huffmanCodes[ch];
        for (char bit : code) {
    compressedOut.push_back(bit == '1');
        }
  }
  deleteTree(root);
  return compressedOut;
}

vector<unsigned char> decode(Node* root, const vector<bool>& compressed) {    // The function for decoding the compressed
      vector<unsigned char> outputDecoded; // Decoded output
      Node* currentNode = root;

      for (bool bit: compressed) {
            if (bit == 0) {
                  currentNode = currentNode->left;
            } else {
                  currentNode = currentNode->right;
            }

            // If a leaf node is reached, add character to output.
            if (currentNode->left == nullptr && currentNode->right == nullptr) {
                  outputDecoded.push_back(currentNode->ch);
                  currentNode = root;   // Reset to root for next character.
            }
      }
      return outputDecoded;
}

void deleteTree(Node* root) {
      if (root) {
            deleteTree(root->left);
            deleteTree(root->right);
            delete root;
      }
}

vector<unsigned char> readBinary(const string& filename) {
      ifstream file (filename, ios::binary);
      return vector<unsigned char>((ifstreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

void writeBinary(const string& filename, const vector<unsigned char>& data) {
      ofstream file(filename, ios::binary);
      file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

int main() {
  crow::SimpleApp app;

  CROW_ROUTE(app, "/compress").methods("POST"_method)([](const crow::request& req) {
    
  }
}
