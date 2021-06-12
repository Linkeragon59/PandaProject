cmake_minimum_required(VERSION 3.16)

add_library(tinyobjloader INTERFACE)

target_include_directories(tinyobjloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/main)
