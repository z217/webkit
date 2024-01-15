cmake_minimum_required(VERSION 3.10)

if (NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if (PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  message(WARNING "The binary directory of CMake is same with source directory")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

set(PROJECT_VERSION 1.0)
project(webkit VERSION ${PROJECT_VERSION})

set(WEBKIT_SOURCE_FILE
  channel/hash_router.h
  channel/simple_adapter.cpp
  channel/simple_adapter.h
  channel/tcp_channel.cpp
  channel/tcp_channel.h
  dispatcher/string_dispatcher.cpp
  dispatcher/string_dispatcher.h
  logger/callback_logger.h
  logger/chain_logger.h
  logger/stdout_logger.h
  logger/comm_logger.h
  packet/byte_packet.cpp
  packet/byte_packet.h
  packet/string_serialization.cpp
  packet/string_serialization.h
  pool/thread_pool.cpp
  pool/thread_pool.h
  reactor/epoll_event.cpp
  reactor/epoll_event.h
  reactor/epoller.cpp
  reactor/epoller.h
  server/thread_server.cpp
  server/thread_server.h
  socket/tcp_socket.cpp
  socket/tcp_socket.h
  util/circular_queue.h
  util/syscall.h
  util/time.h
  util/trace_helper.h
)
add_library(webkit STATIC ${WEBKIT_SOURCE_FILE})

target_include_directories(webkit PUBLIC .)

set(WEBKIT_INCLUDEDIR ${PROJECT_SOURCE_DIR}/include)
target_include_directories(webkit PUBLIC ${WEBKIT_INCLUDEDIR})

add_subdirectory(third_party/fmt)
add_subdirectory(third_party/json)
set(THIRD_PARTY_LIB
  fmt
  nlohmann_json)
target_link_libraries(webkit PUBLIC ${THIRD_PARTY_LIB})

add_subdirectory(example/json_server)

set(WEBKIT_INSTALL_LIBDIR ${PROJECT_SOURCE_DIR}/target/lib)
set(WEBKIT_INSTALL_INCLUDEDIR ${PROJECT_SOURCE_DIR}/target)

set_target_properties(webkit PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${WEBKIT_INSTALL_LIBDIR}
  LIBRARY_OUTPUT_DIRECTORY ${WEBKIT_INSTALL_LIBDIR}
  VERSION ${PROJECT_VERSION}
  SOVERSION 1)

install(TARGETS webkit
  ARCHIVE DESTINATION ${WEBKIT_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${WEBKIT_INSTALL_LIBDIR})
install (DIRECTORY ${WEBKIT_INCLUDEDIR}
  DESTINATION ${WEBKIT_INSTALL_INCLUDEDIR})