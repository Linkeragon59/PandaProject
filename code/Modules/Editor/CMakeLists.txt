cmake_minimum_required(VERSION 3.16)

add_library(Editor "")
target_sources(Editor
	PRIVATE
		Editor_EditorModule.h
		Editor_Precompile.h

		Editor_EditorModule.cpp
)

target_precompile_headers(Editor PRIVATE Editor_Precompile.h)
target_compile_features(Editor PRIVATE cxx_std_20)

target_include_directories(Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Editor GameWork)
