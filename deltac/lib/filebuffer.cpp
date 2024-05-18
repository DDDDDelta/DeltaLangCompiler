#include "filebuffer.hpp"

SourceBuffer::SourceBuffer(std::string_view path_name) : file_path(path_name) {
    std::ifstream ifs(file_path);

    if (!ifs.is_open())
        throw std::ios_base::failure("unable to open file at " + file_path.string());

    /*
    ifs.seekg(0, std::ios::end);
    std::streampos size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    */

    std::uintmax_t size = std::filesystem::file_size(file_path);

    buffer.reserve(size + 1ll);

    char c;
    while ((c = ifs.get()) != EOF) {
        buffer += c;
    }
}

SourceBuffer::SourceBuffer(std::istream& input) {
    buffer.assign((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
}