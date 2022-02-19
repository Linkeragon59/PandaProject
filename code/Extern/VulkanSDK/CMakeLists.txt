cmake_minimum_required(VERSION 3.16)

add_library(VulkanInclude INTERFACE)
add_library(VulkanLib INTERFACE)

target_include_directories(VulkanInclude INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.2.162.1/include)

if ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(VulkanLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.2.162.1/lib/win64)
	target_link_libraries(VulkanLib INTERFACE vulkan-1)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(VulkanLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/1.2.162.1/lib/linux)
	target_link_libraries(VulkanLib INTERFACE vulkan)
endif ()
