cmake_minimum_required(VERSION 3.16)

add_library(Vulkan INTERFACE)

target_include_directories(Vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.2.162.1/include)

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(Vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.2.162.1/lib/linux)
	target_link_libraries(Vulkan INTERFACE vulkan)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(Vulkan INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.2.162.1/lib/win64)
	target_link_libraries(Vulkan INTERFACE vulkan-1)
endif ()
