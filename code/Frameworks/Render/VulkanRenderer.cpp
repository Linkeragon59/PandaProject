#include "VulkanRenderer.h"

#include "VulkanHelpers.h"
#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

#include "VulkanCamera.h"

#include <GLFW/glfw3.h>

namespace Render
{
namespace Vulkan
{
	namespace
	{
		uint32_t locVulkanApiVersion = VK_API_VERSION_1_0;

#if defined(_WINDOWS) && !defined(NDEBUG)
		constexpr bool locEnableValidationLayers = true;
#else
		constexpr bool locEnableValidationLayers = false;
#endif
	}

	Renderer* Renderer::ourInstance = nullptr;

	void Renderer::CreateInstance()
	{
		assert(!ourInstance);
		new Renderer();
	}

	void Renderer::DestroyInstance()
	{
		assert(ourInstance);
		delete ourInstance;
	}

	void Renderer::OnWindowOpened(GLFWwindow* aWindow)
	{
		SwapChain* swapChain = new SwapChain(aWindow);
		mySwapChains.push_back(swapChain);
	}

	void Renderer::OnWindowClosed(GLFWwindow* aWindow)
	{
		for (uint32_t i = 0; i < (uint32_t)mySwapChains.size(); ++i)
		{
			if (mySwapChains[i]->GetWindow() == aWindow)
			{
				delete mySwapChains[i];
				mySwapChains.erase(mySwapChains.begin() + i);
				break;
			}
		}
	}

	void Renderer::Update()
	{
		myCamera->Update();

		for (uint32_t i = 0; i < (uint32_t)mySwapChains.size(); ++i)
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

	Renderer::Renderer()
	{
		CreateVkInstance();

		if (locEnableValidationLayers)
			SetupDebugMessenger();

		CreateDevice();

		ourInstance = this;

		SetupEmptyTexture();

		DeferredPipeline::SetupDescriptorSetLayouts();

		myCamera = new Camera();
		myCamera->SetPosition(glm::vec3(0.0f, 0.75f, -2.0f));
		myCamera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
		myCamera->SetPerspective(800.0f / 600.0f, 60.0f, 0.1f, 256.0f);
	}

	Renderer::~Renderer()
	{
		delete myCamera;
		myCamera = nullptr;

		DeferredPipeline::DestroyDescriptorSetLayouts();

		myMissingTexture.Destroy();

		ourInstance = nullptr;

		delete myDevice;

		if (locEnableValidationLayers)
			DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void Renderer::CreateVkInstance()
	{
		std::vector<const char*> layers;
		if (locEnableValidationLayers)
			PopulateValidationLayers(layers);

		if (!CheckInstanceLayersSupport(layers))
			throw std::runtime_error("Validation layers are enabled but not supported!");

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (locEnableValidationLayers)
			PopulateDebugExtensions(extensions);

		if (!CheckInstanceExtensionsSupport(extensions))
			throw std::runtime_error("Required extensions are not available!");

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
			FillDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &myVkInstance), "Failed to create Vulkan instance!");
	}

	void Renderer::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		FillDebugMessengerCreateInfo(createInfo);

		VK_CHECK_RESULT(CreateDebugMessenger(myVkInstance, &createInfo, nullptr, &myDebugMessenger), "Couldn't create a debug messenger!");
	}

	void Renderer::CreateDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
			throw std::runtime_error("No physical device supporting Vulkan was found!");

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
			PopulateValidationLayers(layers);

		std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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
