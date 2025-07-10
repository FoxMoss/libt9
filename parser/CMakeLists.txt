find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIR})
protobuf_generate_cpp(T9DB T9DB_H ../protobuf/t9db.proto)
protobuf_generate_cpp(SENTENCEPIECE_MODEL SENTENCEPIECE_MODEL_H ../protobuf/sentencepiece_model.proto)
cmake_path(GET T9DB_H PARENT_PATH PROTO_PATH)

add_executable(t9parser main.cc ${T9DB}  ${SENTENCEPIECE_MODEL} )


include(FetchContent)
set(SPM_PROTOBUF_PROVIDER "package")
FetchContent_Declare(
  sentencepiece
  PATH /home/foxmoss/Projects/sentencepiece
  # GIT_REPOSITORY https://github.com/google/sentencepiece/
  # GIT_TAG        master
  # GIT_SHALLOW 1
)
FetchContent_MakeAvailable(sentencepiece)

target_link_libraries(t9parser PRIVATE sentencepiece protobuf::libprotobuf )

target_include_directories(t9parser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${PROTO_PATH}
)

add_custom_command(TARGET t9parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")
