#include "VulkanRender.h"

#include "VulkanHelpers.h"
#include "VulkanShaderHelpers.h"
#include "VulkanDebug.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanglTFModel.h"
#include "VulkanDynamicModel.h"
#include "VulkanRenderer.h"

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
		SetupDefaultData();
	}

	void RenderCore::Finalize()
	{
		Assert(mySwapChains.empty(), "Renderering is being finalized while swapchains are still alive!");
		DeleteUnusedModels(true);

		for (DescriptorContainer& container : myDescriptorContainers)
			container.Destroy();

		DestroyDefaultData();
	}

	void RenderCore::StartFrame()
	{
		DeleteUnusedModels();

		for (SwapChain* swapChain : mySwapChains)
			swapChain->AcquireNext();
	}

	void RenderCore::EndFrame()
	{
		for (SwapChain* swapChain : mySwapChains)
			swapChain->Present();
	}

	void RenderCore::RegisterWindow(GLFWwindow* aWindow, Render::Renderer::Type aType)
	{
		SwapChain* swapChain = new SwapChain(aWindow, aType);
		mySwapChains.push_back(swapChain);
	}

	void RenderCore::UnregisterWindow(GLFWwindow* aWindow)
	{
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			if (mySwapChains[i]->GetWindowHandle() == aWindow)
			{
				delete mySwapChains[i];
				mySwapChains.erase(mySwapChains.begin() + i);
				break;
			}
		}
	}

	Render::Renderer* RenderCore::GetRenderer(GLFWwindow* aWindow)
	{
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			if (mySwapChains[i]->GetWindowHandle() == aWindow)
			{
				return mySwapChains[i]->GetRenderer();
			}
		}
		return nullptr;
	}

	Render::Model* RenderCore::SpawnModel(const BaseModelData& someData)
	{
		switch (someData.GetType())
		{
		case BaseModelData::Type::glTF:
			return new glTF::Model(someData);
		case BaseModelData::Type::Dynamic:
			return new DynamicModel(someData);
		default:
			return nullptr;
		}
	}

	void RenderCore::DespawnModel(Render::Model* aModel)
	{
		myModelsToDelete.push_back(std::make_pair(static_cast<Model*>(aModel), GetModelsLifetime()));
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


	VkDescriptorSetLayout RenderCore::GetDescriptorSetLayout(ShaderHelpers::DescriptorLayout aLayout)
	{
		if (myDescriptorContainers[(size_t)aLayout].myLayout == VK_NULL_HANDLE)
			myDescriptorContainers[(size_t)aLayout].Create(aLayout);

		return myDescriptorContainers[(size_t)aLayout].myLayout;
	}

	void RenderCore::AllocateDescriptorSet(ShaderHelpers::DescriptorLayout aLayout, VkDescriptorSet& anOutDescriptorSet)
	{
		myDescriptorContainers[(size_t)aLayout].AllocateDescriptorSet(anOutDescriptorSet);
	}

	void RenderCore::UpdateDescriptorSet(ShaderHelpers::DescriptorLayout aLayout, const ShaderHelpers::DescriptorInfo& someDescriptorInfo, VkDescriptorSet aDescriptorSet)
	{
		myDescriptorContainers[(size_t)aLayout].UpdateDescriptorSet(someDescriptorInfo, aDescriptorSet);
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

	void RenderCore::SetupDefaultData()
	{
		uint8 white[4] = { 0xff, 0xff, 0xff, 0xff };
		uint8 black[4] = { 0x00, 0x00, 0x00, 0xff };
		uint8 missing[4] = { 0xff, 0x14, 0x93, 0xff };
		SetupTexture(myWhiteTexture, white);
		SetupTexture(myBlackTexture, black);
		SetupTexture(myMissingTexture, missing);

		ShaderHelpers::MaterialData materialData;
		myDefaultMaterial.Create(sizeof(ShaderHelpers::MaterialData),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myDefaultMaterial.SetupDescriptor();
		myDefaultMaterial.Map();
		memcpy(myDefaultMaterial.myMappedData, &materialData, sizeof(ShaderHelpers::MaterialData));
		myDefaultMaterial.Unmap();

		ShaderHelpers::JointMatrixData jointsData;
		myDefaultJointsMatrix.Create(sizeof(ShaderHelpers::JointMatrixData),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myDefaultJointsMatrix.SetupDescriptor();
		myDefaultJointsMatrix.Map();
		memcpy(myDefaultJointsMatrix.myMappedData, &jointsData, sizeof(ShaderHelpers::JointMatrixData));
		myDefaultJointsMatrix.Unmap();
	}

	void RenderCore::DestroyDefaultData()
	{
		myWhiteTexture.Destroy();
		myBlackTexture.Destroy();
		myMissingTexture.Destroy();

		myDefaultMaterial.Destroy();
		myDefaultJointsMatrix.Destroy();
	}

	void RenderCore::SetupTexture(Image& anImage, uint8* aColor)
	{
		anImage.Create(1, 1,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		anImage.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			GetGraphicsQueue());

		Buffer textureStaging;
		textureStaging.Create(4,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		{
			memcpy(textureStaging.myMappedData, aColor, 4);
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
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, anImage.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		EndOneTimeCommand(commandBuffer, GetGraphicsQueue());

		textureStaging.Destroy();

		anImage.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			GetGraphicsQueue());
		anImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		anImage.CreateImageSampler();
		anImage.SetupDescriptor();
	}

	uint RenderCore::GetModelsLifetime() const
	{
		uint lifeTime = 0;
		for (uint i = 0; i < (uint)mySwapChains.size(); ++i)
		{
			lifeTime = std::max(lifeTime, mySwapChains[i]->GetImagesCount());
		}
		return lifeTime;
	}

	void RenderCore::DeleteUnusedModels(bool aDeleteNow)
	{
		for (uint i = 0; i < myModelsToDelete.size();)
		{
			if (aDeleteNow || myModelsToDelete[i].second-- == 0)
			{
				delete myModelsToDelete[i].first;
				myModelsToDelete.erase(myModelsToDelete.begin() + i);
				continue;
			}
			i++;
		}
	}
}
