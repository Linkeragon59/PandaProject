cmake_minimum_required(VERSION 3.6.2)

file(GLOB Headers *.h)
file(GLOB Sources *.cpp)
add_library(Panda STATIC ${Headers} ${Sources})

target_compile_features(Panda PRIVATE cxx_std_20)

target_include_directories(Panda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
