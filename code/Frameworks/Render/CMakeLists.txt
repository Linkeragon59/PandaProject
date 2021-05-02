cmake_minimum_required(VERSION 3.16)

add_library(Render "")
target_sources(Render
	PRIVATE
		DummyModel.cpp
		glTFAnimation.cpp
		glTFMaterial.cpp
		glTFMesh.cpp
		glTFModel.cpp
		glTFNode.cpp
		glTFSkin.cpp
		glTFTexture.cpp
		RenderFacade.cpp
		RenderPrecompile.cpp
		VulkanBuffer.cpp
		VulkanDebugMessenger.cpp
		VulkanDeferredPipeline.cpp
		VulkanDevice.cpp
		VulkanHelpers.cpp
		VulkanImage.cpp
		VulkanRenderer.cpp
		VulkanSwapChain.cpp

		DummyModel.h
		glTFAnimation.h
		glTFMaterial.h
		glTFMesh.h
		glTFModel.h
		glTFNode.h
		glTFSkin.h
		glTFTexture.h
		RenderCamera.h
		RenderFacade.h
		RenderPrecompile.h
		VulkanBuffer.h
		VulkanDebugMessenger.h
		VulkanDeferredPipeline.h
		VulkanDevice.h
		VulkanHelpers.h
		VulkanImage.h
		VulkanModel.h
		VulkanRenderer.h
		VulkanSwapChain.h
)

target_link_libraries(Render Base)

target_compile_features(Render PRIVATE cxx_std_20)

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/stb)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/tinygltf)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/tinyobjloader)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/VulkanMemoryAllocator)

target_precompile_headers(Render PRIVATE RenderPrecompile.h)

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/lib/linux)
	target_link_libraries(Render vulkan)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/lib/win64)
	target_link_libraries(Render vulkan-1)
endif ()
