cmake_minimum_required(VERSION 3.6.2)

file(GLOB Headers *.h)
file(GLOB Sources *.cpp)
file(GLOB Natvis *.natvis)
add_library(Render STATIC ${Headers} ${Sources} ${Natvis})

target_link_libraries(Render Base)

target_compile_features(Render PRIVATE cxx_std_20)

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glm/0.9.9.8/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/stb)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/tinyobjloader)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/VulkanMemoryAllocator)

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/lib/linux)
	target_link_libraries(Render vulkan)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/lib/win64)
	target_link_libraries(Render vulkan-1)
endif ()
