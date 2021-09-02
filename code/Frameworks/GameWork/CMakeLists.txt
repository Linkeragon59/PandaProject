cmake_minimum_required(VERSION 3.16)

add_library(GameWork "")
target_sources(GameWork
	PRIVATE
		GameWork.h
		GameWork_Camera.h
		GameWork_CameraManager.h
		GameWork_ConsoleLog.h
		GameWork_Editor.h
		GameWork_Entity.h
		GameWork_glTFProp.h
		GameWork_GraphEditor.h
		GameWork_PointLight.h
		GameWork_Module.h
		GameWork_Precompile.h
		GameWork_Prop.h
		GameWork_PropManager.h
		GameWork_SimpleGeometryProp.h
		GameWork_WindowManager.h

		GameWork.cpp
		GameWork_Camera.cpp
		GameWork_CameraManager.cpp
		GameWork_ConsoleLog.cpp
		GameWork_Editor.cpp
		GameWork_Entity.cpp
		GameWork_glTFProp.cpp
		GameWork_GraphEditor.cpp
		GameWork_PointLight.cpp
		GameWork_Prop.cpp
		GameWork_PropManager.cpp
		GameWork_SimpleGeometryProp.cpp
		GameWork_WindowManager.cpp
)

target_precompile_headers(GameWork PRIVATE GameWork_Precompile.h)
target_compile_features(GameWork PRIVATE cxx_std_20)

target_include_directories(GameWork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GameWork GLFW)
target_link_libraries(GameWork ImGui)
target_link_libraries(GameWork SoLoud)
target_link_libraries(GameWork Base)
target_link_libraries(GameWork Render)
