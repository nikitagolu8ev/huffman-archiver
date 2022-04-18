#include "iobitstream.h"

using namespace IOBitStream;

BitReader::BitReader(std::istream &in) : input_stream_(in), read_bits_in_current_byte_(0) {}

bool BitReader::GetBit() {
    unsigned char current_byte = input_stream_.get();
    if (!input_stream_.good()) {
        throw std::out_of_range("end of input stream, can't read one more byte");
    }
    bool bit = (current_byte >> read_bits_in_current_byte_) & 1;
    ++read_bits_in_current_byte_;

    if (read_bits_in_current_byte_ == BITS_IN_BYTE) {
        read_bits_in_current_byte_ = 0;
    } else {
        input_stream_.unget();
    }
    return bit;
}

std::vector<bool> BitReader::GetBits(size_t count) {
    std::vector<bool> bits;
    for (size_t i = 0; count > 0 && read_bits_in_current_byte_ != 0; ++i) {
        bits.push_back(GetBit());
        --count;
    }
    while (count >= BITS_IN_BYTE) {
        unsigned char current_byte = input_stream_.get();
        if (!input_stream_.good()) {
            throw std::out_of_range("end of input stream, can't read one more byte");
        }
        for (size_t bit_num = 0; bit_num < BITS_IN_BYTE; ++bit_num) {
            bits.push_back((current_byte >> bit_num) & 1);
        }
        count -= BITS_IN_BYTE;
    }
    while (count > 0) {
        bits.push_back(GetBit());
        --count;
    }
    return bits;
}

BitWriter::BitWriter(std::ostream& out) : output_stream_(out), written_bits_in_current_byte_(0), current_byte_(0) {}

void BitWriter::WriteBit(bool bit) {
    current_byte_ |= static_cast<unsigned char>(bit) << written_bits_in_current_byte_;
    ++written_bits_in_current_byte_;

    if (written_bits_in_current_byte_ == BITS_IN_BYTE) {
        output_stream_.put(static_cast<char>(current_byte_));
        current_byte_ = 0;
        written_bits_in_current_byte_ = 0;
    }
}

void BitWriter::WriteBits(const std::vector<bool>& bits) {
    size_t bit_num = 0;
    while (written_bits_in_current_byte_ != 0 && bit_num < bits.size()) {
        WriteBit(bits[bit_num++]);
    }
    while (bit_num + BITS_IN_BYTE <= bits.size()) {
        for (size_t i = 0; i < BITS_IN_BYTE; ++i) {
            current_byte_ |= (static_cast<unsigned char>(bits[bit_num + i]) << i);
        }
        bit_num += BITS_IN_BYTE;
        output_stream_.put(static_cast<char>(current_byte_));
        current_byte_ = 0;
    }
    while (bit_num < bits.size()) {
        WriteBit(bits[bit_num++]);
    }
}

BitWriter::~BitWriter() {
    if (written_bits_in_current_byte_ != 0) {
        output_stream_.put(static_cast<char>(current_byte_));
    }
}