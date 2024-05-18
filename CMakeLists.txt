cmake_minimum_required(VERSION 3.10)
project(DeltaLangCompiler LANGUAGES CXX)

set(CMAKE_CXX_COMPILER "clang++")

add_subdirectory(deltac)

add_executable(deltac ${SRC_LIST})
