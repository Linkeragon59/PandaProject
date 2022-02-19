cmake_minimum_required(VERSION 3.16)

add_library(Base)
target_sources(Base
	PRIVATE
		public/Base_Assert.h
		public/Base_File.h
		public/Base_glm.h
		public/Base_imgui.h
		public/Base_Input.h
		public/Base_SharedPtr.h
		public/Base_SlotVector.h
		public/Base_Thread.h
		public/Base_Time.h
		public/Base_Types.h
		public/Base_Utils.h
		public/Base_Window.h

		public/glm.natvis

		private/Base_Precompile.h

		private/Base_Assert.cpp
		private/Base_File.cpp
		private/Base_Thread.cpp
		private/Base_Time.cpp
		private/Base_imgui.cpp
		private/Base_Input.cpp
		private/Base_Utils.cpp
		private/Base_Window.cpp
)

target_precompile_headers(Base PRIVATE private/Base_Precompile.h)
target_compile_features(Base PRIVATE cxx_std_23)

target_include_directories(Base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(Base PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(Base PRIVATE GLFWInclude)
target_link_libraries(Base PUBLIC GLFWLib)
target_link_libraries(Base PUBLIC GLM)
target_link_libraries(Base PUBLIC RapidJSON)
target_link_libraries(Base PUBLIC ImGui)
