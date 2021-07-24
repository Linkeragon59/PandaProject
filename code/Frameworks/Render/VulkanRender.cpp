#include "VulkanRender.h"

#include "VulkanHelpers.h"
#include "VulkanDebug.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanDeferredRenderer.h"

#include "VulkanglTFModel.h"

#include <GLFW/glfw3.h>

namespace Render::Vulkan
{
	namespace
	{
		uint locVulkanApiVersion = VK_API_VERSION_1_0;

#if WINDOWS_BUILD && DEBUG_BUILD
		constexpr bool locEnableValidationLayers = true;
#else
		constexpr bool locEnableValidationLayers = false;
#endif
	}

	RenderCore* RenderCore::ourInstance = nullptr;

	void RenderCore::Create()
	{
		Assert(!ourInstance);

		ourInstance = new RenderCore;

		ourInstance->Initialize();
	}

	void RenderCore::Destroy()
	{
		Assert(ourInstance);

		ourInstance->Finalize();

		delete ourInstance;
		ourInstance = nullptr;
	}

	RenderCore::RenderCore()
	{
		CreateVkInstance();

		if (locEnableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			Debug::FillDebugMessengerCreateInfo(createInfo);
			VK_CHECK_RESULT(Debug::CreateDebugMessenger(myVkInstance, &createInfo, nullptr, &myDebugMessenger), "Couldn't create a debug messenger!");
		}

		CreateDevice();

		if (locEnableValidationLayers)
			Debug::SetupDebugMarkers(myDevice->myLogicalDevice);
	}

	RenderCore::~RenderCore()
	{
		delete myDevice;

		if (locEnableValidationLayers)
			Debug::DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void RenderCore::Initialize()
	{
		ShaderHelpers::SetupDescriptorSetLayouts();
		SetupEmptyTexture();
	}

	void RenderCore::Finalize()
	{
		myMissingTexture.Destroy();
		ShaderHelpers::DestroyDescriptorSetLayouts();
	}

	void RenderCore::StartFrame()
	{
		for (SwapChain* swapChain : mySwapChains)
			swapChain->AcquireNext();
	}

	void RenderCore::EndFrame()
	{
		for (SwapChain* swapChain : mySwapChains)
			swapChain->Present();
	}

	void RenderCore::RegisterWindow(GLFWwindow* aWindow)
	{
		SwapChain* swapChain = new SwapChain(aWindow);
		mySwapChains.push_back(swapChain);
	}

	void RenderCore::UnregisterWindow(GLFWwindow* aWindow)
	{
		int index = GetWindowSwapChainIndex(aWindow);
		if (index != -1)
		{
			delete mySwapChains[index];
			mySwapChains.erase(mySwapChains.begin() + index);
		}
	}
	
	SwapChain* RenderCore::GetWindowSwapChain(GLFWwindow* aWindow)
	{
		int index = GetWindowSwapChainIndex(aWindow);
		if (index != -1)
			return mySwapChains[index];
		return nullptr;
	}

	Renderer* RenderCore::CreateRenderer(RendererType aType)
	{
		switch (aType)
		{
		case Render::RendererType::Deferred:
			return new DeferrerRenderer;
		default:
			Assert(false, "Unsupported renderer type")
			return nullptr;
		}
	}

	void RenderCore::DestroyRenderer(Renderer* aRenderer)
	{
		delete aRenderer;
	}

	VkPhysicalDevice RenderCore::GetPhysicalDevice() const
	{
		return myDevice->myPhysicalDevice;
	}

	VkDevice RenderCore::GetDevice() const
	{
		return myDevice->myLogicalDevice;
	}

	VmaAllocator RenderCore::GetAllocator() const
	{
		return myDevice->myVmaAllocator;
	}

	VkQueue RenderCore::GetGraphicsQueue() const
	{
		return myDevice->myGraphicsQueue;
	}

	VkCommandPool RenderCore::GetGraphicsCommandPool() const
	{
		return myDevice->myGraphicsCommandPool;
	}

	uint RenderCore::SpawnModel(const std::string& aFilePath, const Model::RenderData& someRenderData)
	{
		(void)aFilePath;
		(void)someRenderData;
		/*Model* model = nullptr;
		if (aFilePath.empty())
		{
			model = new DummyModel(aRenderData);
		}
		else
		{
			model = new glTF::Model(aFilePath, aRenderData);
		}

		for (uint i = 0; i < (uint)myModels.size(); ++i)
		{
			if (myModels[i] == nullptr)
			{
				myModels[i] = model;
				return i;
			}
		}

		myModels.push_back(model);
		return (uint)myModels.size() - 1;*/
		return 0;
	}

	void RenderCore::DespawnModel(uint anIndex)
	{
		(void)anIndex;
		//myDespawningModels.push_back({ myModels[anIndex], 3 });
		//myModels[anIndex] = nullptr;
	}

	void RenderCore::CreateVkInstance()
	{
		std::vector<const char*> layers;
		if (locEnableValidationLayers)
			Debug::PopulateValidationLayers(layers);

		Verify(CheckInstanceLayersSupport(layers), "Validation layers are enabled but not supported!");

		uint glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (locEnableValidationLayers)
			Debug::PopulateDebugExtensions(extensions);

		Verify(CheckInstanceExtensionsSupport(extensions), "Required extensions are not available!");

		// TODO: Have more parameters when creating the RenderCore
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = locVulkanApiVersion;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		if (locEnableValidationLayers)
		{
			Debug::FillDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}
		createInfo.enabledLayerCount = static_cast<uint>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = static_cast<uint>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &myVkInstance), "Failed to create Vulkan instance!");
	}

	void RenderCore::CreateDevice()
	{
		uint deviceCount = 0;
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, nullptr);
		Assert(deviceCount > 0, "No physical device supporting Vulkan was found!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, devices.data());

		// TODO: Select the physical device based on requirements.
		// For now, just take the first one.
		myDevice = new Device(devices[0]);

		// Choose Device features to enable
		VkPhysicalDeviceFeatures enabledFeatures{};
		if (myDevice->myFeatures.samplerAnisotropy == VK_TRUE)
		{
			enabledFeatures.samplerAnisotropy = VK_TRUE;
		}
		if (myDevice->myFeatures.fillModeNonSolid)
		{
			enabledFeatures.fillModeNonSolid = VK_TRUE;
			if (myDevice->myFeatures.wideLines)
			{
				enabledFeatures.wideLines = VK_TRUE;
			}
		};

		std::vector<const char*> layers;
		if (locEnableValidationLayers)
			Debug::PopulateValidationLayers(layers);

		std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		if (myDevice->SupportsExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

		myDevice->SetupLogicalDevice(
			enabledFeatures,
			layers,
			extensions,
			VK_QUEUE_GRAPHICS_BIT);
		myDevice->SetupVmaAllocator(myVkInstance, locVulkanApiVersion);
	}

	void RenderCore::SetupEmptyTexture()
	{
		myMissingTexture.Create(1, 1,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myMissingTexture.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			GetGraphicsQueue());

		Buffer textureStaging;
		textureStaging.Create(4,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		{
			uint8_t empty[4] = { 0xff, 0x14, 0x93, 0xff };
			memcpy(textureStaging.myMappedData, empty, 4);
		}
		textureStaging.Unmap();

		VkCommandBuffer commandBuffer = BeginOneTimeCommand();
		{
			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { 1, 1, 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, myMissingTexture.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		EndOneTimeCommand(commandBuffer, GetGraphicsQueue());

		textureStaging.Destroy();

		myMissingTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			GetGraphicsQueue());
		myMissingTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myMissingTexture.CreateImageSampler();
		myMissingTexture.SetupDescriptor();
	}

	int RenderCore::GetWindowSwapChainIndex(GLFWwindow* aWindow)
	{
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			if (mySwapChains[i]->GetWindowHandle() == aWindow)
			{
				return i;
			}
		}
		return -1;
	}
}
