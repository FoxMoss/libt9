add_executable(main main.c)

target_link_libraries(main PRIVATE t9)

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
