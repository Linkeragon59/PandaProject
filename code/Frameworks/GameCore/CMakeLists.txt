cmake_minimum_required(VERSION 3.16)

add_library(GameCore)
target_sources(GameCore
	PRIVATE
		public/GameCore.h
		public/GameCore_CallbackGui.h
		public/GameCore_Camera.h
		public/GameCore_CameraManager.h
		public/GameCore_Entity.h
		public/GameCore_glTFProp.h
		public/GameCore_Graph.h
		public/GameCore_Gui.h
		public/GameCore_GuiManager.h
		public/GameCore_PointLight.h
		public/GameCore_Module.h
		public/GameCore_Prop.h
		public/GameCore_PropManager.h
		public/GameCore_SimpleGeometryProp.h

		private/GameCore_Precompile.h
		private/GameCore.cpp
		private/GameCore_CallbackGui.cpp
		private/GameCore_Camera.cpp
		private/GameCore_CameraManager.cpp
		private/GameCore_Entity.cpp
		private/GameCore_glTFProp.cpp
		private/GameCore_Graph.cpp
		private/GameCore_Gui.cpp
		private/GameCore_GuiManager.cpp
		private/GameCore_Module.cpp
		private/GameCore_ModuleManager.h
		private/GameCore_ModuleManager.cpp
		private/GameCore_PointLight.cpp
		private/GameCore_Prop.cpp
		private/GameCore_PropManager.cpp
		private/GameCore_SimpleGeometryProp.cpp
)

target_precompile_headers(GameCore PRIVATE private/GameCore_Precompile.h)
target_compile_features(GameCore PRIVATE cxx_std_23)

target_include_directories(GameCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(GameCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(GameCore PRIVATE SoLoud)
target_link_libraries(GameCore PUBLIC Base)
target_link_libraries(GameCore PUBLIC Render)
target_link_libraries(GameCore PRIVATE GLFWInclude)
target_link_libraries(GameCore PUBLIC GLFWLib)
