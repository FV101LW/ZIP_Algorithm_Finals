const express = require('express');
const multer = require('multer');
const fs = require('fs');
const path = require('path');

// Node for Huffman tree
class Node {
    constructor(char, freq) {
        this.char = char;
        this.freq = freq;
        this.left = null;
        this.right = null;
    }
}

// Comparator for priority queue
class Compare {
    static compare(a, b) {
        return a.freq - b.freq;
    }
}

// Function to build the Huffman tree
function buildTree(freqMap) {
    const heap = Object.entries(freqMap).map(([char, freq]) => new Node(char, freq));
    
    heap.sort(Compare.compare);
    
    while (heap.length > 1) {
        const left = heap.shift();
        const right = heap.shift();
        
        const merged = new Node(null, left.freq + right.freq);
        merged.left = left;
        merged.right = right;
        heap.push(merged);
        
        heap.sort(Compare.compare);
    }

    return heap[0];
}

// Function to generate Huffman codes
function generateCodes(node, code = '', codes = {}) {
    if (!node) return;
    if (node.char !== null) codes[node.char] = code;
    
    generateCodes(node.left, code + '0', codes);
    generateCodes(node.right, code + '1', codes);
    
    return codes;
}

// Function to pack bits into bytes
function packBits(bits) {
    const bytes = [];
    let current = 0;
    let count = 0;

    for (const b of bits) {
        current = (current << 1) | b;
        count++;
        if (count === 8) {
            bytes.push(current);
            count = 0;
            current = 0;
        }
    }
    if (count > 0) {
        current <<= (8 - count);
        bytes.push(current);
    }
    return Buffer.from(bytes);
}

// Function to compress data
function compress(input) {
    const freq = {};
    for (const c of input) {
        freq[c] = (freq[c] || 0) + 1;
    }

    const root = buildTree(freq);
    const codes = generateCodes(root);

    const bits = [];
    for (const c of input) {
        for (const bit of codes[c]) {
            bits.push(bit === '1' ? 1 : 0);
        }
    }

    return packBits(bits);
}

// Set up Express server
const app = express();
const upload = multer({ dest: 'uploads/' });

app.post('/compress', upload.single('file'), (req, res) => {
    if (!req.file) {
        return res.status(400).send('No file uploaded');
    }

    const filePath = path.join(__dirname, req.file.path);
    const data = fs.readFileSync(filePath);
    const compressed = compress([...data]);

    res.setHeader('Content-Type', 'application/octet-stream');
    res.setHeader('Content-Disposition', 'attachment; filename="compressed.huff"');
    res.send(compressed);

    // Clean up uploaded file
    fs.unlinkSync(filePath);
});

// Start the server
const PORT = 18080;
app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
