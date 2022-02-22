cmake_minimum_required(VERSION 3.16)

add_library(ImGui)
target_sources(ImGui
	PRIVATE
		1.83/imconfig.h
		1.83/imgui.h
		1.83/imgui_internal.h
		1.83/imstb_rectpack.h
		1.83/imstb_textedit.h
		1.83/imstb_truetype.h

		1.83/imgui.cpp
		1.83/imgui_demo.cpp
		1.83/imgui_draw.cpp
		1.83/imgui_tables.cpp
		1.83/imgui_widgets.cpp
)

target_compile_features(ImGui PRIVATE cxx_std_23)

target_include_directories(ImGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/1.83)
