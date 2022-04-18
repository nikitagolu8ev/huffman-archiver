#include "huffman.h"

#include <fstream>

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "-h") {
        std::cerr << std::endl;
        std::cerr << argv[0] << " -c  archive_name file1 [file2 ...]          zip files \"file1\", \"file2\", ... "
                                "in file \"archive_name\"" << std::endl;
        std::cerr << argv[0] << " -d archive_name                             unzip file \"archive_name\"" << std::endl;
        std::cerr << argv[0] << " -h                                          help" << std::endl;
    } else if (argc == 3 &&std::string(argv[1]) == "-d") {
        try {
            Huffman::Decoder decoder;
            decoder.Decode(static_cast<std::string>(argv[2]));
        } catch (std::runtime_error& error) {
            std::cerr << "Error : " << error.what() << std::endl;
            return 1;
        }
    } else if (argc > 3 && std::string(argv[1]) == "-c") {
        std::ofstream archive_file_stream(std::string(argv[2]), std::ios::binary);
        try {
            Huffman::Encoder encoder(archive_file_stream);
            for (size_t i = 3; i < argc; ++i) {
                std::string file_name(argv[i]);
                std::ifstream input_file_stream(file_name, std::ios::binary);
                if (!input_file_stream.is_open()) {
                    throw std::runtime_error("file named \"" + file_name + "\" doesn't exist");
                }
                encoder.AddFile(file_name, input_file_stream);
            }
        } catch (std::runtime_error& error) {
            std::cerr << "Error : " << error.what() << std::endl;
            return 1;
        }
    } else {
        std::cerr << "No such command. For help type \"" << argv[0] << " -h\"" << std::endl;
    }
    return 0;
}
