cmake_minimum_required(VERSION 2.8)

add_executable(tcp_server ${NET_SERVER_SOURCE} tcp/server.cpp)
target_link_libraries(tcp_server pthread)

add_executable(tcp_client ${NET_SERVER_SOURCE} tcp/client.cpp)
target_link_libraries(tcp_client pthread)