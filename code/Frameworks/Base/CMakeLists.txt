cmake_minimum_required(VERSION 3.16)

add_library(Base "")
target_sources(Base
	PRIVATE
		Assert.h
		BasePrecompile.h
		File.h
		glmInclude.h
		Input.h
		Thread.h
		Types.h

		Assert.cpp
		File.cpp
		Thread.cpp
		Input.cpp

		glm.natvis
)

target_precompile_headers(Base PRIVATE BasePrecompile.h)
target_compile_features(Base PRIVATE cxx_std_20)

target_include_directories(Base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Base GLFW)
target_link_libraries(Base GLM)
