cmake_minimum_required(VERSION 3.6.2)

file(GLOB Headers *.h)
file(GLOB Sources *.cpp)
add_library(Base STATIC ${Headers} ${Sources})

target_compile_features(Base PRIVATE cxx_std_20)

target_include_directories(Base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Base PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/include)

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(Base PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/lib/linux)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(Base PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/lib/win64)
endif ()

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_libraries(Base glfw3)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_libraries(Base glfw3)
endif ()
