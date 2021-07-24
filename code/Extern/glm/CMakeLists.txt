cmake_minimum_required(VERSION 3.16)

add_library(GLM INTERFACE)

target_include_directories(GLM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/0.9.9.8/include)
