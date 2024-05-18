#pragma once

#include <iostream>
#include <fstream>
#include <string_view>
#include <filesystem>

class SourceBuffer {
public:
    using ConstIter = std::string::const_iterator;

public:
    explicit SourceBuffer(std::string_view path_name) noexcept(false);
    explicit SourceBuffer(std::istream& input);
    ConstIter cbegin() const { return buffer.cbegin(); }
    ConstIter cend() const { return buffer.cend(); }
    const char* ptr_cbegin() const { return &buffer.front(); }
    // points the the next position passing null terminate character
    const char* ptr_cend() const { return &buffer.back() + 2; }
    std::size_t size() const { return buffer.size(); }
    std::string name() const { return file_path.filename().string(); }

private:
    std::string buffer;
    std::filesystem::path file_path;
};