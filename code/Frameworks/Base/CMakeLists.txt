cmake_minimum_required(VERSION 3.16)

add_library(Base "")
target_sources(Base
	PRIVATE
		Base_Assert.h
		Base_File.h
		Base_glm.h
		Base_imgui.h
		Base_Input.h
		Base_Precompile.h
		Base_SharedPtr.h
		Base_SlotVector.h
		Base_Thread.h
		Base_Time.h
		Base_Types.h
		Base_Utils.h
		Base_Window.h

		Base_Assert.cpp
		Base_File.cpp
		Base_Thread.cpp
		Base_Time.cpp
		Base_imgui.cpp
		Base_Input.cpp
		Base_Window.cpp

		glm.natvis
)

target_precompile_headers(Base PRIVATE Base_Precompile.h)
target_compile_features(Base PRIVATE cxx_std_20)

target_include_directories(Base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Base GLFW)
target_link_libraries(Base GLM)
target_link_libraries(Base ImGui)
