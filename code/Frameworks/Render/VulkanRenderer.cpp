#include "VulkanRenderer.h"

#include "RenderFacade.h"

#include "VulkanHelpers.h"
#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanCamera.h"

#include "DummyModel.h"
#include "VulkanglTFModel.h"

#include <GLFW/glfw3.h>

namespace Render
{
namespace Vulkan
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

	Renderer* Renderer::GetInstance()
	{
		return Facade::GetInstance()->GetRenderer();
	}

	Renderer::Renderer()
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

	Renderer::~Renderer()
	{
		delete myDevice;

		if (locEnableValidationLayers)
			Debug::DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void Renderer::Init()
	{
		ShaderHelpers::SetupDescriptorSetLayouts();
		SetupEmptyTexture();

		myCamera = new Camera;
	}

	void Renderer::Finalize()
	{
		delete myCamera;
		myCamera = nullptr;

		for (uint i = 0; i < (uint)myModels.size(); ++i)
		{
			if (myModels[i])
				delete myModels[i];
		}
		myModels.clear();

		for (uint i = 0; i < (uint)myDespawningModels.size(); ++i)
		{
			delete myDespawningModels[i].myModel;
		}
		myDespawningModels.clear();

		myMissingTexture.Destroy();
		ShaderHelpers::DestroyDescriptorSetLayouts();
	}

	void Renderer::OnWindowOpened(GLFWwindow* aWindow)
	{
		SwapChain* swapChain = new SwapChain(aWindow);
		mySwapChains.push_back(swapChain);
	}

	void Renderer::OnWindowClosed(GLFWwindow* aWindow)
	{
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			if (mySwapChains[i]->GetWindow() == aWindow)
			{
				delete mySwapChains[i];
				mySwapChains.erase(mySwapChains.begin() + i);
				break;
			}
		}
	}

	void Renderer::Update(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myCamera->Update(aView, aProjection);

		// TODO: This is slow, need optimization
		for (uint i = 0; i < (uint)myModels.size(); ++i)
		{
			if (myModels[i])
				myModels[i]->Update();
		}

		for (int i = (int)myDespawningModels.size() - 1; i >= 0; --i)
		{
			if (--myDespawningModels[i].myFramesToKeep == 0)
			{
				delete myDespawningModels[i].myModel;
				myDespawningModels.erase(myDespawningModels.begin() + i);
			}
		}

		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			mySwapChains[i]->Update();
		}
	}

	VkPhysicalDevice Renderer::GetPhysicalDevice() const
	{
		return myDevice->myPhysicalDevice;
	}

	VkDevice Renderer::GetDevice() const
	{
		return myDevice->myLogicalDevice;
	}

	VmaAllocator Renderer::GetAllocator() const
	{
		return myDevice->myVmaAllocator;
	}

	VkQueue Renderer::GetGraphicsQueue() const
	{
		return myDevice->myGraphicsQueue;
	}

	VkCommandPool Renderer::GetGraphicsCommandPool() const
	{
		return myDevice->myGraphicsCommandPool;
	}

	uint Renderer::SpawnModel(const std::string& aFilePath, const RenderData& aRenderData)
	{
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			mySwapChains[i]->SetDirty();
		}

		Model* model = nullptr;
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
		return (uint)myModels.size() - 1;
	}

	void Renderer::DespawnModel(uint anIndex)
	{
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			mySwapChains[i]->SetDirty();
		}

		myDespawningModels.push_back({ myModels[anIndex], 3 });
		myModels[anIndex] = nullptr;
	}

	void Renderer::CreateVkInstance()
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

	void Renderer::CreateDevice()
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

	void Renderer::SetupEmptyTexture()
	{
		myMissingTexture.Create(1, 1,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myMissingTexture.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			Renderer::GetInstance()->GetGraphicsQueue());

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
		EndOneTimeCommand(commandBuffer, Renderer::GetInstance()->GetGraphicsQueue());

		textureStaging.Destroy();

		myMissingTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			Renderer::GetInstance()->GetGraphicsQueue());
		myMissingTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myMissingTexture.CreateImageSampler();
		myMissingTexture.SetupDescriptor();
	}
}
}
