cmake_minimum_required(VERSION 3.6.2)

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

target_compile_features(Render PRIVATE cxx_std_20)

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glm/0.9.9.8/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/stb)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/tinyobjloader)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/ktx/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/ktx/other_include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/tinygltf)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/include)
target_include_directories(Render PRIVATE ${CMAKE_SOURCE_DIR}/code/Extern/VulkanMemoryAllocator)

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/lib/linux)
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/lib/linux)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/glfw/3.3/lib/win64)
	target_link_directories(Render PUBLIC ${CMAKE_SOURCE_DIR}/code/Extern/VulkanSDK/1.2.162.1/lib/win64)
endif ()

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	target_link_libraries(Render glfw3 vulkan pthread dl X11)
elseif ( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	target_link_libraries(Render glfw3 vulkan-1)
endif ()
