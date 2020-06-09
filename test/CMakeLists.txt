cmake_minimum_required(VERSION 2.8)

add_executable(tcp_server ${NET_SERVER_SOURCE} tcp/server.cpp)
target_link_libraries(tcp_server pthread)

add_executable(tcp_client ${NET_SERVER_SOURCE} tcp/client.cpp)
target_link_libraries(tcp_client pthread)

add_executable(virsual_rkaiq_tool ${NET_SERVER_SOURCE} virsual_rkaiq_tool/virsual_rkaiq_tool.cpp)
target_link_libraries(virsual_rkaiq_tool pthread)

add_executable(frame_average ${NET_SERVER_SOURCE} frame_average/frame_average.cpp)
target_link_libraries(frame_average pthread)