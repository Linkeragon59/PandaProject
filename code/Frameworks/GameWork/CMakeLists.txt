cmake_minimum_required(VERSION 3.16)

file(GLOB Headers *.h)
file(GLOB Sources *.cpp)
add_library(GameWork STATIC ${Headers} ${Sources})

target_link_libraries(GameWork Base)
target_link_libraries(GameWork Render)

target_compile_features(GameWork PRIVATE cxx_std_20)

target_include_directories(GameWork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(GameWork PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/include)
