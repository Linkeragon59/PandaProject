cmake_minimum_required(VERSION 3.16)

add_library(Render)
target_sources(Render
	PRIVATE
		public/Render_Camera.h
		public/Render_Core.h
		public/Render_Debug.h
		public/Render_DeferredPipeline.h
		public/Render_DeferredRenderer.h
		public/Render_DescriptorContainer.h
		public/Render_EditorRenderer.h
		public/Render_Facade.h
		public/Render_glTFAnimation.h
		public/Render_glTFMaterial.h
		public/Render_glTFMesh.h
		public/Render_glTFModel.h
		public/Render_glTFNode.h
		public/Render_glTFSkin.h
		public/Render_glTFTexture.h
		public/Render_Gui.h
		public/Render_GuiContainer.h
		public/Render_GuiPipeline.h
		public/Render_Handle.h
		public/Render_Helpers.h
		public/Render_Light.h
		public/Render_Model.h
		public/Render_ModelData.h
		public/Render_ModelContainer.h
		public/Render_PointLightsSet.h
		public/Render_Renderer.h
		public/Render_RendererImpl.h
		public/Render_Resource.h
		public/Render_ShaderHelpers.h
		public/Render_SimpleGeometryModel.h
		public/Render_SwapChain.h
		public/Render_VulkanBuffer.h
		public/Render_VulkanDevice.h
		public/Render_VulkanImage.h

		private/Render_Precompile.h

		private/Render_Camera.cpp
		private/Render_Core.cpp
		private/Render_Debug.cpp
		private/Render_DeferredPipeline.cpp
		private/Render_DeferredRenderer.cpp
		private/Render_DescriptorContainer.cpp
		private/Render_EditorRenderer.cpp
		private/Render_Facade.cpp
		private/Render_glTFAnimation.cpp
		private/Render_glTFMaterial.cpp
		private/Render_glTFMesh.cpp
		private/Render_glTFModel.cpp
		private/Render_glTFNode.cpp
		private/Render_glTFSkin.cpp
		private/Render_glTFTexture.cpp
		private/Render_Gui.cpp
		private/Render_GuiContainer.cpp
		private/Render_GuiPipeline.cpp
		private/Render_Helpers.cpp
		private/Render_ModelData.cpp
		private/Render_ModelContainer.cpp
		private/Render_PointLightsSet.cpp
		private/Render_Precompile.cpp
		private/Render_Renderer.cpp
		private/Render_RendererImpl.cpp
		private/Render_Resource.cpp
		private/Render_ShaderHelpers.cpp
		private/Render_SimpleGeometryModel.cpp
		private/Render_SwapChain.cpp
		private/Render_VulkanBuffer.cpp
		private/Render_VulkanDevice.cpp
		private/Render_VulkanImage.cpp
)

target_precompile_headers(Render PRIVATE private/Render_Precompile.h)
target_compile_features(Render PRIVATE cxx_std_23)

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(Render PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(Render PUBLIC tinyglTF)
target_link_libraries(Render PRIVATE VulkanMemoryAllocator)
target_link_libraries(Render PRIVATE VulkanInclude)
target_link_libraries(Render PUBLIC VulkanLib)
target_link_libraries(Render PRIVATE Base)
target_link_libraries(Render PRIVATE GLFWInclude)
target_link_libraries(Render PUBLIC GLFWLib)
