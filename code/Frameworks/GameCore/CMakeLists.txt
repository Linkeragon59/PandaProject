cmake_minimum_required(VERSION 3.16)

add_library(GameCore)
target_sources(GameCore
	PRIVATE
		public/GameCore_Assert.h
		public/GameCore_Defines.h
		public/GameCore_Entity.h
		public/GameCore_EntityCameraComponent.h
		public/GameCore_EntityModule.h
		public/GameCore_EntityTransformComponent.h
		public/GameCore_Facade.h
		public/GameCore_File.h
		public/GameCore_glm.h
		public/GameCore_Graph.h
		public/GameCore_imgui.h
		public/GameCore_InputModule.h
		public/GameCore_Module.h
		public/GameCore_SharedPtr.h
		public/GameCore_SlotVector.h
		public/GameCore_Thread.h
		public/GameCore_TimeModule.h
		public/GameCore_Utils.h
		public/GameCore_WindowModule.h
		public/glm.natvis

		private/GameCore_Precompile.h
		private/GameCore_Assert.cpp
		private/GameCore_EntityCameraComponent.cpp
		private/GameCore_EntityModule.cpp
		private/GameCore_Facade.cpp
		private/GameCore_File.cpp
		private/GameCore_Graph.cpp
		private/GameCore_imgui.cpp
		private/GameCore_InputModule.cpp
		private/GameCore_Module.cpp
		private/GameCore_ModuleManager.h
		private/GameCore_ModuleManager.cpp
		private/GameCore_Thread.cpp
		private/GameCore_TimeModule.cpp
		private/GameCore_Utils.cpp
		private/GameCore_WindowModule.cpp
		
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
