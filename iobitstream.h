#pragma once

#include <iostream>
#include <vector>

namespace IOBitStream {
    const size_t BITS_IN_BYTE = 8;

    class BitReader {
    public:
        BitReader(std::istream& in);

        bool GetBit();
        std::vector<bool> GetBits(size_t count);

    private:
        std::istream& input_stream_;
        size_t read_bits_in_current_byte_;
    };

    class BitWriter {
    public:
        BitWriter(std::ostream &out);

        void WriteBit(bool bit);
        void WriteBits(const std::vector<bool>& bits);

        ~BitWriter();

    private:
        unsigned char current_byte_;
        size_t written_bits_in_current_byte_;
        std::ostream& output_stream_;
    };
}
