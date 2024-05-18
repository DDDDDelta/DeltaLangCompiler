add_library(deltac_lib
    lib/lexer.cpp
    lib/filebuffer.cpp
    lib/keywordtrie.cpp
    lib/charinfo.cpp
    lib/tokentype.cpp
)

target_include_directories(deltac_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
