enable_testing()
add_executable(lexer_tests lexer_tests.cpp)
target_include_directories(lexer_tests PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_link_libraries(lexer_tests deltac_lib gtest gtest_main)