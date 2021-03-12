cmake_minimum_required(VERSION 3.16)

file(GLOB Headers *.h)
file(GLOB Sources *.cpp)
add_library(Panda STATIC ${Headers} ${Sources})

target_compile_features(Panda PRIVATE cxx_std_20)

target_link_libraries(Panda Base)
target_link_libraries(Panda Render)
target_link_libraries(Panda GameWork)

target_include_directories(Panda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
