cmake_minimum_required(VERSION 3.16)

add_library(Render "")
target_sources(Render
	PRIVATE
		RenderFacade.h
		RenderLight.h
		RenderModel.h
		RenderRenderer.h
		RenderPrecompile.h
		VulkanBuffer.h
		VulkanCamera.h
		VulkanDebug.h
		VulkanDeferredPipeline.h
		VulkanDeferredRenderer.h
		VulkanDescriptorContainer.h
		VulkanDevice.h
		VulkanglTFAnimation.h
		VulkanglTFMaterial.h
		VulkanglTFMesh.h
		VulkanglTFModel.h
		VulkanglTFNode.h
		VulkanglTFSkin.h
		VulkanglTFTexture.h
		VulkanGui.h
		VulkanGuiPipeline.h
		VulkanGuiRenderer.h
		VulkanHelpers.h
		VulkanImage.h
		VulkanImGuiOverlay.h
		VulkanModel.h
		VulkanPointLightsSet.h
		VulkanRender.h
		VulkanRenderer.h
		VulkanShaderHelpers.h
		VulkanSimpleGeometryModel.h
		VulkanSwapChain.h

		RenderFacade.cpp
		RenderModel.cpp
		RenderPrecompile.cpp
		VulkanBuffer.cpp
		VulkanCamera.cpp
		VulkanDebug.cpp
		VulkanDeferredPipeline.cpp
		VulkanDeferredRenderer.cpp
		VulkanDescriptorContainer.cpp
		VulkanDevice.cpp
		VulkanglTFAnimation.cpp
		VulkanglTFMaterial.cpp
		VulkanglTFMesh.cpp
		VulkanglTFModel.cpp
		VulkanglTFNode.cpp
		VulkanglTFSkin.cpp
		VulkanglTFTexture.cpp
		VulkanGui.cpp
		VulkanGuiPipeline.cpp
		VulkanGuiRenderer.cpp
		VulkanHelpers.cpp
		VulkanImage.cpp
		VulkanImGuiOverlay.cpp
		VulkanPointLightsSet.cpp
		VulkanRender.cpp
		VulkanRenderer.cpp
		VulkanShaderHelpers.cpp
		VulkanSimpleGeometryModel.cpp
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
