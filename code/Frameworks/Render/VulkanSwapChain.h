#pragma once

#include "VulkanImage.h"
#include "VulkanDeferredContext.h"

struct GLFWwindow;

namespace Render
{
namespace Vulkan
{
	class Camera;

	class SwapChain
	{
	public:
		SwapChain(GLFWwindow* aWindow);
		~SwapChain();

		void Setup();
		void Cleanup();
		void Recreate();

		void UpdateView(const glm::mat4& aView, const glm::mat4& aProjection);
		void Update();

		GLFWwindow* GetWindowHandle() const { return myWindow; }

	private:
		static void FramebufferResizedCallback(GLFWwindow* aWindow, int aWidth, int aHeight);

		void SetupVkSwapChain();
		void CleanupVkSwapChain();

		void SetupDepthStencil();
		void CleanupDepthStencil();

		void CreateSyncObjects();
		void DestroySyncObjects();

		void SetupCommandBuffers();
		void CleanupCommandBuffers();

		void DrawFrame();

		GLFWwindow* myWindow = nullptr;
		VkSurfaceKHR mySurface = VK_NULL_HANDLE;
		VkDevice myDevice = VK_NULL_HANDLE;
		bool myFramebufferResized = false;

		VkSwapchainKHR myVkSwapChain = VK_NULL_HANDLE;
		std::vector<Image> myImages;
		Image myDepthImage;

		std::vector<VkCommandBuffer> myCommandBuffers;

		RenderContextDeferred myDeferredRenderContext;

		// One per in flight frame
		uint myMaxInFlightFrames = 0;
		uint myCurrentInFlightFrame = 0;
		std::vector<VkSemaphore> myImageAvailableSemaphores;
		std::vector<VkSemaphore> myRenderFinishedSemaphores;
		std::vector<VkFence> myInFlightFrameFences;

		Camera* myCamera = nullptr;

		// Lights will be moved to a separate class later
		static const uint ourNumLights = 64;
		struct Light
		{
			glm::vec4 myPosition;
			glm::vec3 myColor;
			float myRadius;
		};
		struct LightData
		{
			glm::vec4 myViewPos;
			Light myLights[ourNumLights];
		} myLightsData;
		Buffer myLightsUBO;
		VkDescriptorPool myLightsDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet myLightsDescriptorSet = VK_NULL_HANDLE;
		void UpdateLightsUBO();
		void SetupRandomLights();
		void SetupLightsDescriptorPool();
		void SetupLightsDescriptorSets();
	};
}
}
