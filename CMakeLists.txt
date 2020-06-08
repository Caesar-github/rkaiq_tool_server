cmake_minimum_required(VERSION 2.8)

project(rkaiq_tool_server)

include_directories(
	${PROJECT_SOURCE_DIR}/
	${PROJECT_SOURCE_DIR}/netserver/
	${PROJECT_SOURCE_DIR}/rkaiq/
	${PROJECT_SOURCE_DIR}/camera/)

add_definitions(-std=c++11)

aux_source_directory(${PROJECT_SOURCE_DIR}/netserver NET_SERVER_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/rkaiq RKAIQ_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/camera/ CAMERA_SOURCE)

option(BUILD_SANITIZER_STATIC "compile with sanitizer (static library linker)" OFF)
if(BUILD_SANITIZER_STATIC)
	add_definitions(-fsanitize=address -static-libasan -g -ggdb -gdwarf -funwind-tables -rdynamic -O0)
	link_libraries(libasan.a dl m rt)
endif()

option(BUILD_TEST "build test demo" OFF)
if(BUILD_TEST)
	add_subdirectory(test)
endif()

file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h)
add_custom_target(
    clangformat
    COMMAND /usr/bin/clang-format
    -style=LLVM
    -i
    ${ALL_SOURCE_FILES})

set(RKAIQ_TOOL_SERVER_SOURCE
	${NET_SERVER_SOURCE}
	${RKAIQ_SOURCE}
	${CAMERA_SOURCE}
	rkaiq_tool_server.cpp)
add_executable(rkaiq_tool_server ${RKAIQ_TOOL_SERVER_SOURCE})
target_link_libraries(rkaiq_tool_server pthread)