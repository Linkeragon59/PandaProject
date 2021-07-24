#pragma once

struct GLFWwindow;

namespace Render
{
	class SwapChain
	{
	public:
		SwapChain(GLFWwindow* aWindow);
		virtual ~SwapChain();

		virtual void Setup() = 0;
		virtual void Cleanup() = 0;
		virtual void Recreate() = 0;

		virtual void AcquireNext() = 0;
		virtual void Present() = 0;

		GLFWwindow* GetWindowHandle() const { return myWindow; }

	protected:
		static void FramebufferResizedCallback(GLFWwindow* aWindow, int aWidth, int aHeight);

		GLFWwindow* myWindow = nullptr;
		bool myFramebufferResized = false;
	};
}
