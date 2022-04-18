#pragma once

#include "huffman.h"

#include <vector>

namespace Huffman {
    class HuffmanTrie;
}

class Heap {
public:
    struct Element {
        size_t count;
        Huffman::HuffmanTrie *huffman_trie;
    };

    void Insert(const Element& element);
    void ExtractMin();

    const Element& GetMin() const;

    size_t Size() const;
    bool IsEmpty() const;

private:
    void PushDown();
    void PushUp();

    static size_t Parent(size_t vertex);
    static size_t LeftSon(size_t vertex);
    static size_t RightSon(size_t vertex);

    std::vector<Element> elements_;
};

