cmake_minimum_required(VERSION 3.10)
project(DeltaLangCompiler LANGUAGES CXX)

#set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${CMAKE_SOURCE_DIR}/deltac/include)

add_subdirectory(deltac)

add_subdirectory(test)

add_executable(deltac testmain.cpp)

target_link_libraries(deltac deltac_lib)
