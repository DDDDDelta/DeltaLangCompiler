#include "filebuffer.hpp"

namespace deltac {

SourceBuffer::SourceBuffer(std::string_view path_name) : file_path(path_name) {
    std::ifstream ifs(file_path);

    if (!ifs.is_open()) {
        std::cerr << "cannot openfile";
        std::exit(2);
    }

    buffer.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

SourceBuffer::SourceBuffer(std::istream& input) {
    buffer.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
}

}
