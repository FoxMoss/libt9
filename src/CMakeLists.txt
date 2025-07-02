set(SOURCES trie.c)

if(ENABLE_WORDLIST)
    message(STATUS "Including wordlist feature")
    list(APPEND SOURCES wordlist.c)
else()
    message(STATUS "Wordlist feature not included")
endif()

add_library(t9 ${SOURCES})

target_include_directories(t9 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(TARGET t9 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")
