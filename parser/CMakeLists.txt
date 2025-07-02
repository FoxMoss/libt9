add_executable(t9parser main.cc)

target_link_libraries(t9parser PUBLIC sentencepiece)

target_include_directories(t9parser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
