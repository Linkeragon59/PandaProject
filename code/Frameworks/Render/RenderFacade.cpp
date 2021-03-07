#include "RenderFacade.h"

#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

#ifdef __linux__
#pragma GCC diagnostic pop
#endif

#if defined(_WINDOWS)
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4324)
#elif defined(__linux__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#if defined(_WINDOWS)
#pragma warning(pop)
#elif defined(__linux__)
#pragma GCC diagnostic pop
#endif

#include <GLFW/glfw3.h>

#include "TriangleRenderer.h"
#include "BasicRendererTuto.h"
#include "BasicRenderer.h"

#include "VulkanRenderCore.h"

namespace Render
{
	TriangleRenderer* Facade::ourTriangleRenderer = nullptr;
	BasicRendererTuto* Facade::ourBasicRendererTuto = nullptr;
	BasicRenderer* Facade::ourBasicRenderer = nullptr;

	void Facade::RunTriangleRenderer()
	{
		ourTriangleRenderer = new TriangleRenderer();
		ourTriangleRenderer->Run();
		delete ourTriangleRenderer;
	}

	void Facade::RunBasicRendererTuto()
	{
		ourBasicRendererTuto = new BasicRendererTuto();
		ourBasicRendererTuto->Run();
		delete ourBasicRendererTuto;
	}

	void Facade::RunBasicRenderer()
	{
		ourBasicRenderer = new BasicRenderer();
		ourBasicRenderer->Run();
		delete ourBasicRenderer;
	}

	void Facade::RunVulkanRenderer()
	{
		VulkanRenderCore::CreateInstance();
		GLFWwindow* window = VulkanRenderCore::GetInstance()->OpenWindow(800, 600, "Test");

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			VulkanRenderCore::GetInstance()->Update();
		}

		VulkanRenderCore::GetInstance()->CloseWindow(window);
		VulkanRenderCore::DestroyInstance();
	}

}
