#include "DummyRenderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

namespace Render
{
	DummyRenderer::DummyRenderer()
		: myWindow(nullptr)
	{
		glfwInit();
	}
	
	DummyRenderer::~DummyRenderer()
	{
		glfwTerminate();
	}
	
	void DummyRenderer::OpenWindow()
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		myWindow = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
	}
	
	void DummyRenderer::PrintExtensionsCount()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::cout << extensionCount << " extensions supported\n";
	}
	
	void DummyRenderer::TestMatrixCalculation()
	{
		glm::mat4 matrix;
		glm::vec4 vec;
		auto test = matrix * vec;
		(void)test;
	}
	
	void DummyRenderer::Run()
	{
		while (!glfwWindowShouldClose(myWindow))
		{
			glfwPollEvents();
		}
	}
	
	void DummyRenderer::CloseWindow()
	{
		glfwDestroyWindow(myWindow);
	}
}