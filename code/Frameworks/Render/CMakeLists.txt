cmake_minimum_required(VERSION 3.16)

add_library(Render "")
target_sources(Render
	PRIVATE
		DummyModel.h
		RenderData.h
		RenderFacade.h
		RenderPrecompile.h
		VulkanBuffer.h
		VulkanCamera.h
		VulkanDebugMessenger.h
		VulkanDeferredPipeline.h
		VulkanDeferredContext.h
		VulkanDevice.h
		VulkanglTFAnimation.h
		VulkanglTFMaterial.h
		VulkanglTFMesh.h
		VulkanglTFModel.h
		VulkanglTFNode.h
		VulkanglTFSkin.h
		VulkanglTFTexture.h
		VulkanHelpers.h
		VulkanImage.h
		VulkanImGuiOverlay.h
		VulkanModel.h
		VulkanRenderer.h
		VulkanShaderHelpers.h
		VulkanSwapChain.h

		DummyModel.cpp
		RenderData.cpp
		RenderFacade.cpp
		RenderPrecompile.cpp
		VulkanBuffer.cpp
		VulkanCamera.cpp
		VulkanDebugMessenger.cpp
		VulkanDeferredPipeline.cpp
		VulkanDeferredContext.cpp
		VulkanDevice.cpp
		VulkanglTFAnimation.cpp
		VulkanglTFMaterial.cpp
		VulkanglTFMesh.cpp
		VulkanglTFModel.cpp
		VulkanglTFNode.cpp
		VulkanglTFSkin.cpp
		VulkanglTFTexture.cpp
		VulkanHelpers.cpp
		VulkanImage.cpp
		VulkanImGuiOverlay.cpp
		VulkanRenderer.cpp
		VulkanShaderHelpers.cpp
		VulkanSwapChain.cpp
)

target_precompile_headers(Render PRIVATE RenderPrecompile.h)
target_compile_features(Render PRIVATE cxx_std_20)

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Render GLFW)
target_link_libraries(Render GLM)
target_link_libraries(Render ImGui)
target_link_libraries(Render tinyglTF)
target_link_libraries(Render VulkanMemoryAllocator)
target_link_libraries(Render Vulkan)
target_link_libraries(Render Base)
