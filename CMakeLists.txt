cmake_minimum_required(VERSION 3.9)
project(tcp_server)

set(CMAKE_CXX_STANDARD 17)

link_libraries(pthread)

add_executable(tcp_server src/main.cc src/tcp/server.cc src/tcp/peer.cc)