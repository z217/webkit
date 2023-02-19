set (JSON_SERVER_SOURCE_FILE
  json_server_client.cpp
  json_server_client.h  
  json_server_dispatcher.cpp
  json_server_dispatcher.h
  main.cpp
  server_impl.cpp
  server_impl.h
)

add_executable(json_server ${JSON_SERVER_SOURCE_FILE})

target_link_libraries(json_server webkit)

set(JSON_SERVER_TARGET_BIN_DIR ${PROJECT_SOURCE_DIR}/target/bin/example/json_server)
set_target_properties(json_server PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${JSON_SERVER_TARGET_BIN_DIR})