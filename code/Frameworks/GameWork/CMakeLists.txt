cmake_minimum_required(VERSION 3.16)

add_library(GameWork)
target_sources(GameWork
	PRIVATE
		public/GameWork.h
		public/GameWork_CallbackGui.h
		public/GameWork_Camera.h
		public/GameWork_CameraManager.h
		public/GameWork_Entity.h
		public/GameWork_glTFProp.h
		public/GameWork_Graph.h
		public/GameWork_Gui.h
		public/GameWork_GuiManager.h
		public/GameWork_PointLight.h
		public/GameWork_Module.h
		public/GameWork_Prop.h
		public/GameWork_PropManager.h
		public/GameWork_SimpleGeometryProp.h

		private/GameWork_Precompile.h

		private/GameWork.cpp
		private/GameWork_CallbackGui.cpp
		private/GameWork_Camera.cpp
		private/GameWork_CameraManager.cpp
		private/GameWork_Entity.cpp
		private/GameWork_glTFProp.cpp
		private/GameWork_Graph.cpp
		private/GameWork_Gui.cpp
		private/GameWork_GuiManager.cpp
		private/GameWork_PointLight.cpp
		private/GameWork_Prop.cpp
		private/GameWork_PropManager.cpp
		private/GameWork_SimpleGeometryProp.cpp
)

target_precompile_headers(GameWork PRIVATE private/GameWork_Precompile.h)
target_compile_features(GameWork PRIVATE cxx_std_23)

target_include_directories(GameWork PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(GameWork PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(GameWork PRIVATE SoLoud)
target_link_libraries(GameWork PUBLIC Base)
target_link_libraries(GameWork PUBLIC Render)
target_link_libraries(GameWork PRIVATE GLFWInclude)
target_link_libraries(GameWork PUBLIC GLFWLib)
