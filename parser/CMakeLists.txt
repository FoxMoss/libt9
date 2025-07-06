add_executable(t9parser main.cc sentencepiece_model.pb.cc ../protobuf/t9db.pb.cc)

include(FetchContent)
FetchContent_Declare(
  sentencepiece
  PATH /home/foxmoss/Projects/sentencepiece
  # GIT_REPOSITORY https://github.com/google/sentencepiece/
  # GIT_TAG        master
  # GIT_SHALLOW 1
)
FetchContent_MakeAvailable(sentencepiece)

target_link_libraries(t9parser sentencepiece protobuf)

target_include_directories(t9parser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)
