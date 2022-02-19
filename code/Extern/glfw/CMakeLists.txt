cmake_minimum_required(VERSION 3.16)

add_library(GLFWInclude INTERFACE)
add_library(GLFWLib INTERFACE)

target_include_directories(GLFWInclude INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3.3/include)

if ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(GLFWLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3.3/lib/win64)
	target_link_libraries(GLFWLib INTERFACE glfw3)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(GLFWLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/3.3/lib/linux)
	target_link_libraries(GLFWLib INTERFACE glfw3 pthread dl X11)
endif ()
