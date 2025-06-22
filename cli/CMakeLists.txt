add_executable(main main.c)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

target_link_libraries(main PRIVATE t9 ${CURSES_LIBRARIES})

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
