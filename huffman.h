#pragma once

#include "iobitstream.h"
#include "heap.h"

#include <bitset>
#include <unordered_map>
#include <vector>

class Heap;

namespace Huffman {

    const size_t SYMBOL_SIZE = 9;

    using Symbol = std::bitset<SYMBOL_SIZE>;
    using HuffmanCode = std::vector<bool>;

    const Symbol FILENAME_END = 256;
    const Symbol ONE_MORE_FILE = 257;
    const Symbol ARCHIVE_END = 258;

    class HuffmanTrie {
    public:
        HuffmanTrie();

        explicit HuffmanTrie(const Symbol& symbol);

        static HuffmanTrie Merge(HuffmanTrie& left, HuffmanTrie& right);

        std::vector<std::pair<Symbol, size_t>> GetSymbolsWithCodeLengths() const;

        void AddSymbol(const Symbol& symbol, const HuffmanCode& code);

        Symbol GetNextSymbol(IOBitStream::BitReader& bit_reader) const;

        ~HuffmanTrie();

    private:
        struct Node {
        public:
            Node* left;
            Node* right;
            Symbol symbol;

            Node();

            explicit Node(const Symbol& symbol);

            ~Node();
        };

        Node* root_;
    };

    class Encoder {
    public:
        explicit Encoder(std::ostream& out);

        void AddFile(const std::string& file_name, std::istream& in);

        ~Encoder();

    private:
        IOBitStream::BitWriter bit_writer_;
        std::unordered_map<Symbol, HuffmanCode> codes_of_symbols_;

        static std::vector<std::pair<Symbol, HuffmanCode>> GetSymbolsWithCodes(const std::vector<Symbol>& text_to_code);

        void PrintEncodedFile(const std::vector<std::pair<Symbol, HuffmanCode>>& symbols_with_codes,
                              const std::vector<Symbol>& text_to_code);
    };

    class Decoder {
    public:
        void Decode(const std::string& archive_name);

    private:
        static std::unordered_map<Symbol, HuffmanCode> GetCodesOfSymbols(IOBitStream::BitReader& bit_reader);
    };
}