add_library(t9protocol t9db.pb-c.c)
target_link_libraries(t9protocol PRIVATE protobuf-c)

