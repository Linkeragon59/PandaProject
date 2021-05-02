cmake_minimum_required(VERSION 3.16)

add_library(Base "")
target_sources(Base
	PRIVATE
		Assert.cpp
		File.cpp
		Input.cpp

		Assert.h
		File.h
		Input.h

		glm.natvis
)

target_compile_features(Base PRIVATE cxx_std_20)

target_include_directories(Base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Base PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/glm/0.9.9.8/include)
target_include_directories(Base PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/include)

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(Base PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/lib/linux)
	target_link_libraries(Base glfw3 pthread dl X11)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(Base PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/lib/win64)
	target_link_libraries(Base glfw3)
endif ()
