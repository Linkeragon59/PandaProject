cmake_minimum_required(VERSION 3.16)

add_library(GameWork "")
target_sources(GameWork
	PRIVATE
		Camera.h
		CameraManager.h
		Entity.h
		GameWork.h
		GameWorkPrecompile.h
		glTFProp.h
		Gui.h
		PointLight.h
		Module.h
		Prop.h
		PropManager.h
		SimpleGeometryProp.h
		WindowManager.h

		Camera.cpp
		CameraManager.cpp
		Entity.cpp
		GameWork.cpp
		glTFProp.cpp
		Gui.cpp
		PointLight.cpp
		Prop.cpp
		PropManager.cpp
		SimpleGeometryProp.cpp
		WindowManager.cpp
)

target_precompile_headers(GameWork PRIVATE GameWorkPrecompile.h)
target_compile_features(GameWork PRIVATE cxx_std_20)

target_include_directories(GameWork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GameWork GLFW)
target_link_libraries(GameWork ImGui)
target_link_libraries(GameWork SoLoud)
target_link_libraries(GameWork Base)
target_link_libraries(GameWork Render)
