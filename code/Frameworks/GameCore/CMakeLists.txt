cmake_minimum_required(VERSION 3.16)

add_library(GameCore)
target_sources(GameCore
	PRIVATE
		public/GameCore_Assert.h
		public/GameCore_File.h
		public/GameCore_glm.h
		public/GameCore_imgui.h
		public/GameCore_Input.h
		public/GameCore_SharedPtr.h
		public/GameCore_SlotVector.h
		public/GameCore_Thread.h
		public/GameCore_TimeModule.h
		public/GameCore_Defines.h
		public/GameCore_Utils.h
		public/GameCore_Window.h
		public/GameCore_Facade.h
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
		public/glm.natvis

		private/GameCore_Precompile.h
		private/GameCore_Assert.cpp
		private/GameCore_File.cpp
		private/GameCore_Thread.cpp
		private/GameCore_TimeModule.cpp
		private/GameCore_imgui.cpp
		private/GameCore_Input.cpp
		private/GameCore_Utils.cpp
		private/GameCore_Window.cpp
		private/GameCore_Facade.cpp
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

target_link_libraries(GameCore PUBLIC GLM)
target_link_libraries(GameCore PUBLIC GLFWLib)

target_link_libraries(GameCore PRIVATE GLFWInclude)
target_link_libraries(GameCore PRIVATE SoLoud)
target_link_libraries(GameCore PRIVATE RapidJSON)
target_link_libraries(GameCore PRIVATE ImGui)
