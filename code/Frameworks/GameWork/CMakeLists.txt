cmake_minimum_required(VERSION 3.16)

add_library(GameWork "")
target_sources(GameWork
	PRIVATE
		Camera.h
		GameWork.h
		GameWorkPrecompile.h
		Module.h
		Prop.h

		Camera.cpp
		GameWork.cpp
		Prop.cpp
)

target_precompile_headers(GameWork PRIVATE GameWorkPrecompile.h)
target_compile_features(GameWork PRIVATE cxx_std_20)

target_include_directories(GameWork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GameWork GLFW)
target_link_libraries(GameWork Base)
target_link_libraries(GameWork Render)
