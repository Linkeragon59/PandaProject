cmake_minimum_required(VERSION 3.16)

add_library(Editor)
target_sources(Editor
	PRIVATE
		public/Editor_EditorModule.h

		private/Editor_Precompile.h
		private/Editor_GraphEditorCanvas.h

		private/Editor_EditorModule.cpp
		private/Editor_GraphEditorCanvas.cpp
)

target_precompile_headers(Editor PRIVATE private/Editor_Precompile.h)
target_compile_features(Editor PRIVATE cxx_std_23)

target_include_directories(Editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(Editor PRIVATE GameCore)
target_link_libraries(Editor PRIVATE Render)
target_link_libraries(Editor PRIVATE GLFWInclude)
target_link_libraries(Editor PRIVATE ImGui)
target_link_libraries(Editor PRIVATE RapidJSON)
