#pragma once

#include "VulkanImage.h"

struct GLFWwindow;

namespace Render
{
namespace glTF
{
	class Model;
}
namespace Vulkan
{
	struct Device;
	class SwapChain;

	class Camera;
	class DummyModel;

	class Renderer
	{
	public:
		static void CreateInstance();
		static void DestroyInstance();
		static Renderer* GetInstance() { return ourInstance; }

		void OnWindowOpened(GLFWwindow* aWindow);
		void OnWindowClosed(GLFWwindow* aWindow);

		void Update();

		VkInstance GetVkInstance() const { return myVkInstance; }

		Device* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

		const VkDescriptorImageInfo* GetMissingTextureDescriptor() const { return &myMissingTexture.myDescriptor; }

		Camera* GetCamera() const { return myCamera; }

	private:
		static Renderer* ourInstance;

		Renderer();
		~Renderer();

		void CreateVkInstance();
		void SetupDebugMessenger();
		void CreateDevice();

		void SetupEmptyTexture();

		VkInstance myVkInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;

		Device* myDevice = nullptr;

		// One SwapChain per window/surface
		std::vector<SwapChain*> mySwapChains;

		Image myMissingTexture;

		Camera* myCamera = nullptr;
	};
}
}
