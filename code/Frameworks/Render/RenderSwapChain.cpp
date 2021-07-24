#include "RenderSwapChain.h"

#include <GLFW/glfw3.h>

namespace Render
{

	SwapChain::SwapChain(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		glfwSetWindowUserPointer(myWindow, this);
		glfwSetFramebufferSizeCallback(myWindow, FramebufferResizedCallback);
	}

	SwapChain::~SwapChain()
	{
	}

	void SwapChain::FramebufferResizedCallback(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		(void)aWidth;
		(void)aHeight;
		auto app = reinterpret_cast<SwapChain*>(glfwGetWindowUserPointer(aWindow));
		app->myFramebufferResized = true;
	}
}
