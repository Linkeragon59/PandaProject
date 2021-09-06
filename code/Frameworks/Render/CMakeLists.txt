cmake_minimum_required(VERSION 3.16)

add_library(Render "")
target_sources(Render
	PRIVATE
		Render_Camera.h
		Render_Core.h
		Render_Debug.h
		Render_DeferredPipeline.h
		Render_DeferredRenderer.h
		Render_DescriptorContainer.h
		Render_EditorRenderer.h
		Render_Facade.h
		Render_glTFAnimation.h
		Render_glTFMaterial.h
		Render_glTFMesh.h
		Render_glTFModel.h
		Render_glTFNode.h
		Render_glTFSkin.h
		Render_glTFTexture.h
		Render_Gui.h
		Render_GuiContainer.h
		Render_GuiPipeline.h
		Render_Handle.h
		Render_Helpers.h
		Render_Light.h
		Render_Model.h
		Render_ModelData.h
		Render_ModelContainer.h
		Render_PointLightsSet.h
		Render_Precompile.h
		Render_Renderer.h
		Render_RendererImpl.h
		Render_Resource.h
		Render_ShaderHelpers.h
		Render_SimpleGeometryModel.h
		Render_SwapChain.h
		Render_VulkanBuffer.h
		Render_VulkanDevice.h
		Render_VulkanImage.h

		Render_Camera.cpp
		Render_Core.cpp
		Render_Debug.cpp
		Render_DeferredPipeline.cpp
		Render_DeferredRenderer.cpp
		Render_DescriptorContainer.cpp
		Render_EditorRenderer.cpp
		Render_Facade.cpp
		Render_glTFAnimation.cpp
		Render_glTFMaterial.cpp
		Render_glTFMesh.cpp
		Render_glTFModel.cpp
		Render_glTFNode.cpp
		Render_glTFSkin.cpp
		Render_glTFTexture.cpp
		Render_Gui.cpp
		Render_GuiContainer.cpp
		Render_GuiPipeline.cpp
		Render_Helpers.cpp
		Render_ModelData.cpp
		Render_ModelContainer.cpp
		Render_PointLightsSet.cpp
		Render_Precompile.cpp
		Render_Renderer.cpp
		Render_RendererImpl.cpp
		Render_Resource.cpp
		Render_ShaderHelpers.cpp
		Render_SimpleGeometryModel.cpp
		Render_SwapChain.cpp
		Render_VulkanBuffer.cpp
		Render_VulkanDevice.cpp
		Render_VulkanImage.cpp
)

target_precompile_headers(Render PRIVATE Render_Precompile.h)
target_compile_features(Render PRIVATE cxx_std_20)

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Render GLFW)
target_link_libraries(Render GLM)
target_link_libraries(Render ImGui)
target_link_libraries(Render tinyglTF)
target_link_libraries(Render VulkanMemoryAllocator)
target_link_libraries(Render Vulkan)
target_link_libraries(Render Base)
