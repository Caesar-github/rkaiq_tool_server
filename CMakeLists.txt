cmake_minimum_required(VERSION 2.8)

project(rkaiq_tool_server C CXX)

include_directories(
	${PROJECT_SOURCE_DIR}/
	${PROJECT_SOURCE_DIR}/netserver/
	${PROJECT_SOURCE_DIR}/rkaiq/
	${PROJECT_SOURCE_DIR}/camera/
	${PROJECT_SOURCE_DIR}/mediactl/)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti ${CMAKE_CXX_FLAGS}")
endif(CMAKE_COMPILER_IS_GNUCXX)

aux_source_directory(${PROJECT_SOURCE_DIR}/logger/ LOGGER_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/netserver NET_SERVER_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/rkaiq RKAIQ_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/camera/ CAMERA_SOURCE)
aux_source_directory(${PROJECT_SOURCE_DIR}/mediactl/ MEDIACTL_SOURCE)

option(BUILD_SANITIZER_STATIC "compile with sanitizer (static library linker)" OFF)
if(BUILD_SANITIZER_STATIC)
	add_definitions(-fsanitize=address -static-libasan -g -ggdb -gdwarf -funwind-tables -rdynamic -O0 )
	link_libraries(libasan.a dl m rt)
endif()

option(BUILD_RKAIQ_API "depend on rkaiq api library" ON)
if(BUILD_RKAIQ_API)
aux_source_directory(${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api RKAIQ_SOURCE)
find_path(RKAIQ_API_INCLUDE_DIR
    NAMES rk_aiq_user_api_imgproc.h
    HINTS ${CMAKE_FIND_ROOT_PATH}
    PATH_SUFFIXES rkaiq/uAPI)
include_directories(${RKAIQ_API_INCLUDE_DIR}/)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../xcore)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../common)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../algos)
include_directories(${RKAIQ_API_INCLUDE_DIR}/../iq_parser)
include_directories(${PROJECT_SOURCE_DIR}/rkaiq/rkaiq_api)
link_libraries(rkaiq)
add_definitions(-DENABLE_RKAIQ_UAPI)
endif()

option(BUILD_RKMEDIA "depend on rkmeda library" ON)
if(BUILD_RKMEDIA)
find_package(PkgConfig QUIET)
pkg_check_modules(PKG_EASYMEDIA QUIET "libeasymedia")
set(EASYMEDIA_DEFINITIONS ${PKG_EASYMEDIA_CFLAGS_OTHER})
find_path(EASYMEDIA_INCLUDE_DIR
    NAMES buffer.h
    HINTS ${PKG_EASYMEDIA_INCLUDEDIR} ${PKG_EASYMEDIA_INCLUDE_DIRS}
    PATH_SUFFIXES easymedia)
include_directories(${PROJECT_SOURCE_DIR}/rkmedia/)
link_libraries(easymedia)
add_definitions(-DENABLE_RSTP_SERVER)
include_directories(${EASYMEDIA_INCLUDE_DIR}/)
aux_source_directory(${PROJECT_SOURCE_DIR}/rkmedia/ RKMEDIA_SOURCE)
endif()

file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.cc *.h)
add_custom_target(
    clangformat
    COMMAND /usr/bin/clang-format
    -style=LLVM
    -i
    ${ALL_SOURCE_FILES})

set(RKAIQ_TOOL_SERVER_SOURCE
	${LOGGER_SOURCE}
	${NET_SERVER_SOURCE}
	${RKAIQ_SOURCE}
	${CAMERA_SOURCE}
	${RKMEDIA_SOURCE}
	${MEDIACTL_SOURCE}
	rkaiq_tool_server.cpp)

add_executable(rkaiq_tool_server ${RKAIQ_TOOL_SERVER_SOURCE})
target_link_libraries(rkaiq_tool_server pthread)
install(TARGETS rkaiq_tool_server RUNTIME DESTINATION bin)

option(BUILD_TEST "build test demo" OFF)
if(BUILD_TEST)
	add_subdirectory(test)
endif()
