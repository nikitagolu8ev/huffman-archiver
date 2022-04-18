#include "heap.h"
#include "huffman.h"

#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>

using namespace Huffman;

HuffmanTrie::HuffmanTrie() {
    root_ = nullptr;
}

HuffmanTrie::HuffmanTrie(const Symbol& symbol) {
    root_ = new Node(symbol);
}

HuffmanTrie HuffmanTrie::Merge(HuffmanTrie& left, HuffmanTrie& right) {
    HuffmanTrie merged_trie;
    merged_trie.root_ = new Node;
    merged_trie.root_->left = left.root_;
    merged_trie.root_->right = right.root_;
    return merged_trie;
}

std::vector<std::pair<Symbol, size_t>> HuffmanTrie::GetSymbolsWithCodeLengths() const {
    std::vector<std::pair<Symbol, size_t>> symbols_with_code_lengths;
    if (root_ != nullptr) {
        std::queue<std::pair<Node*, size_t>> trie_bfs;
        trie_bfs.emplace(root_, 0);
        while (!trie_bfs.empty()) {
            auto [current_vertex, current_size] = trie_bfs.front();
            if (!current_vertex->left && !current_vertex->right) {
                symbols_with_code_lengths.emplace_back(current_vertex->symbol, current_size);
            } else {
                if (current_vertex->left) {
                    trie_bfs.emplace(current_vertex->left, current_size + 1);
                }
                if (current_vertex->right) {
                    trie_bfs.emplace(current_vertex->right, current_size + 1);
                }
            }
            trie_bfs.pop();
        }
    }

    return symbols_with_code_lengths;
}

void HuffmanTrie::AddSymbol(const Symbol& symbol, const HuffmanCode& code) {
    if (root_ == nullptr) {
        root_ = new Node;
    }
    Node* current_vertex = root_;
    for (auto bit : code) {
        if (bit) {
            if (current_vertex->right == nullptr) {
                current_vertex->right = new Node;
            }
            current_vertex = current_vertex->right;
        } else {
            if (current_vertex->left == nullptr) {
                current_vertex->left = new Node;
            }
            current_vertex = current_vertex->left;
        }
    }
    current_vertex->symbol = symbol;
}

Symbol HuffmanTrie::GetNextSymbol(IOBitStream::BitReader& bit_reader) const {
    Node* current_vertex = root_;
    if (current_vertex == nullptr) {
        throw std::out_of_range("trying to get symbol from empty trie");
    }
    while (current_vertex->left != nullptr || current_vertex->right != nullptr) {
        if (bit_reader.GetBit()) {
            current_vertex = current_vertex->right;
        } else {
            current_vertex = current_vertex->left;
        }
        if (current_vertex == nullptr) {
            throw std::out_of_range("next symbol doesn't exist in trie");
        }
    }
    return current_vertex->symbol;
}

HuffmanTrie::~HuffmanTrie() {
    delete root_;
}

HuffmanTrie::Node::Node() : left(nullptr), right(nullptr) {}

HuffmanTrie::Node::Node(const Symbol &symbol) : symbol(symbol), left(nullptr), right(nullptr) {}

HuffmanTrie::Node::~Node() {
    delete left;
    delete right;
}

Encoder::Encoder(std::ostream& out) : bit_writer_(out) {}

void Encoder::AddFile(const std::string& file_name, std::istream& input_stream) {
    if (!codes_of_symbols_.empty()) {
        bit_writer_.WriteBits(codes_of_symbols_[ONE_MORE_FILE]);
    }

    std::vector<Symbol> text_to_code;
    for (auto& symbol: file_name) {
        text_to_code.emplace_back(symbol);
    }
    text_to_code.emplace_back(FILENAME_END);

    Symbol current_symbol = input_stream.get();
    while (!input_stream.eof()) {
        text_to_code.push_back(current_symbol);
        current_symbol = input_stream.get();
    }

    text_to_code.emplace_back(ONE_MORE_FILE);
    text_to_code.emplace_back(ARCHIVE_END);

    auto symbols_with_codes = GetSymbolsWithCodes(text_to_code);
    text_to_code.erase(text_to_code.end() - 2, text_to_code.end());

    PrintEncodedFile(symbols_with_codes, text_to_code);
}

Encoder::~Encoder() {
    if (!codes_of_symbols_.empty()) {
        bit_writer_.WriteBits(codes_of_symbols_[ARCHIVE_END]);
    }
}

std::vector<std::pair<Symbol, HuffmanCode>> Encoder::GetSymbolsWithCodes(const std::vector<Symbol>& text_to_code) {
    std::unordered_map<Symbol, size_t> symbol_quantity_in_file;
    for (auto& symbol: text_to_code) {
        ++symbol_quantity_in_file[symbol];
    }

    Heap huffman_heap;
    for (auto& [symbol, count]: symbol_quantity_in_file) {
        huffman_heap.Insert({count, new HuffmanTrie(symbol)});
    }
    while (huffman_heap.Size() != 1) {
        auto [count_left, left_sub_trie] = huffman_heap.GetMin();
        huffman_heap.ExtractMin();
        auto [count_right, right_sub_trie] = huffman_heap.GetMin();
        huffman_heap.ExtractMin();

        huffman_heap.Insert({count_left + count_right,
                             new HuffmanTrie(HuffmanTrie::Merge(*left_sub_trie, *right_sub_trie))});
    }

    std::vector<std::pair<Symbol, size_t>> symbols_with_code_lengths = huffman_heap.GetMin().huffman_trie->GetSymbolsWithCodeLengths();
    delete huffman_heap.GetMin().huffman_trie;

    std::sort(symbols_with_code_lengths.begin(), symbols_with_code_lengths.end(),
              [](const std::pair<Symbol, size_t>& a, const std::pair<Symbol, size_t>& b) {
        return a.second < b.second;
    });

    auto HuffmanCodeIncrement = [](HuffmanCode &code) {
        size_t i = code.size();
        while (i != 0 && code[i - 1]) {
            code[--i] = false;
        }
        if (i == 0) {
            code.insert(code.begin(), true);
        } else {
            code[i - 1] = true;
        }
    };

    std::vector<std::pair<Symbol, HuffmanCode>> symbols_with_codes;
    HuffmanCode current_code;
    for (auto& [symbol, code_size] : symbols_with_code_lengths) {
        while (current_code.size() != code_size) {
            current_code.push_back(false);
        }
        symbols_with_codes.emplace_back(symbol, current_code);
        HuffmanCodeIncrement(current_code);
    }
    return symbols_with_codes;
}

void Encoder::PrintEncodedFile(const std::vector<std::pair<Symbol, HuffmanCode>>& symbols_with_codes,
                               const std::vector<Symbol>& text_to_code) {
    auto ToBits = [](const Symbol &symbol) {
        std::vector<bool> bits(SYMBOL_SIZE);
        for (size_t i = 0; i < SYMBOL_SIZE; ++i) {
            bits[i] = symbol[i];
        }
        return bits;
    };

    bit_writer_.WriteBits(ToBits(symbols_with_codes.size()));
    std::vector<size_t> symbols_size_count(symbols_with_codes.back().second.size(), 0);
    codes_of_symbols_.clear();

    for (auto& [symbol, code] : symbols_with_codes) {
        bit_writer_.WriteBits(ToBits(symbol));
        ++symbols_size_count[code.size() - 1];
        codes_of_symbols_[symbol] = code;
    }
    for (size_t i = 0; i < symbols_with_codes.back().second.size(); ++i) {
        bit_writer_.WriteBits(ToBits(symbols_size_count[i]));
    }

    for (auto &symbol : text_to_code) {
        bit_writer_.WriteBits(codes_of_symbols_[symbol]);
    }
}

void Decoder::Decode(const std::string& archive_name) {
    std::ifstream input_archive_stream(archive_name, std::ios::binary);
    IOBitStream::BitReader bit_reader(input_archive_stream);

    bool last_file = false;

    while (!last_file) {
        try {
            auto codes_of_symbols = GetCodesOfSymbols(bit_reader);
            HuffmanTrie huffman_trie;
            for (auto& [symbol, code]: codes_of_symbols) {
                huffman_trie.AddSymbol(symbol, code);
            }

            std::string file_name;
            auto current_symbol = huffman_trie.GetNextSymbol(bit_reader);
            while (current_symbol != FILENAME_END) {
                file_name += static_cast<char>(current_symbol.to_ulong());
                current_symbol = huffman_trie.GetNextSymbol(bit_reader);
            }

            std::ofstream output_file_stream(file_name, std::ios::binary);
            current_symbol = huffman_trie.GetNextSymbol(bit_reader);
            while (current_symbol != ONE_MORE_FILE && current_symbol != ARCHIVE_END) {
                output_file_stream.put(static_cast<char>(current_symbol.to_ulong()));
                current_symbol = huffman_trie.GetNextSymbol(bit_reader);
            }

            last_file = current_symbol == ARCHIVE_END;
        } catch (std::out_of_range&) {
            throw std::runtime_error("invalid file, can't decode encoded data");
        }
    }
}

std::unordered_map<Symbol, HuffmanCode> Decoder::GetCodesOfSymbols(IOBitStream::BitReader& bit_reader) {
    auto ToNumber = [](const std::vector<bool> &bits) {
        size_t num = 0;
        for (size_t i = 0; i < bits.size(); ++i) {
            num |= static_cast<size_t>(bits[i]) << i;
        }
        return num;
    };

    size_t symbols_count = ToNumber(bit_reader.GetBits(9));
    std::vector<Symbol> symbols;
    for (size_t i = 0; i < symbols_count; ++i) {
        symbols.emplace_back(ToNumber(bit_reader.GetBits(9)));
    }
    size_t current_symbols_count = 0;
    std::vector<size_t> symbols_size_count;
    while (current_symbols_count != symbols_count) {
        symbols_size_count.push_back(ToNumber(bit_reader.GetBits(9)));
        current_symbols_count += symbols_size_count.back();
    }

    auto HuffmanCodeIncrement = [](HuffmanCode &code) {
        size_t i = code.size();
        while (i != 0 && code[i - 1]) {
            code[--i] = false;
        }
        if (i == 0) {
            code.insert(code.begin(), true);
        } else {
            code[i - 1] = true;
        }
    };

    std::unordered_map<Symbol, HuffmanCode> codes_of_symbols;
    HuffmanCode current_code = {false};
    size_t symbols_with_current_code_size = 0;
    for (auto& symbol : symbols) {
        while (symbols_with_current_code_size == symbols_size_count[current_code.size() - 1]) {
            symbols_with_current_code_size = 0;
            current_code.push_back(false);
        }
        codes_of_symbols[symbol] = current_code;
        ++symbols_with_current_code_size;
        HuffmanCodeIncrement(current_code);
    }

    return codes_of_symbols;
}