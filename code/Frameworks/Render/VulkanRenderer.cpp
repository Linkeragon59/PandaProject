#include "VulkanRenderer.h"

#include "VulkanHelpers.h"
#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanPSOContainer.h"

#include "VulkanCamera.h"
#include "VulkanModel.h"
#include "glTFModel.h"

#include <GLFW/glfw3.h>

namespace Render
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

	VulkanRenderer* VulkanRenderer::ourInstance = nullptr;

	void VulkanRenderer::CreateInstance()
	{
		assert(!ourInstance);
		new VulkanRenderer();
	}

	void VulkanRenderer::DestroyInstance()
	{
		assert(ourInstance);
		delete ourInstance;
	}

	void VulkanRenderer::OnWindowOpened(GLFWwindow* aWindow)
	{
		VulkanSwapChain* swapChain = new VulkanSwapChain(aWindow);
		mySwapChains.push_back(swapChain);
	}

	void VulkanRenderer::OnWindowClosed(GLFWwindow* aWindow)
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

	void VulkanRenderer::Update()
	{
		myCamera->Update();
		for (VulkanModel* model : myPandaModels)
			model->Update();
		myglTFModel->Update();

		for (uint32_t i = 0; i < (uint32_t)mySwapChains.size(); ++i)
		{
			mySwapChains[i]->Update();
		}
	}

	VkPhysicalDevice VulkanRenderer::GetPhysicalDevice() const
	{
		return myDevice->myPhysicalDevice;
	}

	VkDevice VulkanRenderer::GetDevice() const
	{
		return myDevice->myLogicalDevice;
	}

	VmaAllocator VulkanRenderer::GetAllocator() const
	{
		return myDevice->myVmaAllocator;
	}

	VkQueue VulkanRenderer::GetGraphicsQueue() const
	{
		return myDevice->myGraphicsQueue;
	}

	VkCommandPool VulkanRenderer::GetGraphicsCommandPool() const
	{
		return myDevice->myGraphicsCommandPool;
	}

	VulkanRenderer::VulkanRenderer()
	{
		CreateVkInstance();

		if (locEnableValidationLayers)
			SetupDebugMessenger();

		CreateDevice();

		ourInstance = this;

		SetupEmptyTexture();

		VulkanPSOContainer::SetupDescriptorSetLayouts();

		myCamera = new VulkanCamera();
		myPandaModels.push_back(new VulkanModel(glm::vec3(0.f)));
		myPandaModels.push_back(new VulkanModel(glm::vec3(0.f, 1.f, 1.f)));
		myglTFModel = new glTFModel();
		//myglTFModel->LoadFromFile("Frameworks/models/treasure_smooth.gltf", myDevice->myGraphicsQueue, 1.0f);
		//myglTFModel->LoadFromFile("Frameworks/models/RiggedFigure/RiggedFigure.gltf", myDevice->myGraphicsQueue, 1.0f);
		myglTFModel->LoadFromFile("Frameworks/models/Avocado/Avocado.gltf", myDevice->myGraphicsQueue, 50.0f);
		//myglTFModel->LoadFromFile("Frameworks/models/CesiumMan/CesiumMan.gltf", myDevice->myGraphicsQueue, 50.0f);
	}

	VulkanRenderer::~VulkanRenderer()
	{
		delete myCamera;
		myCamera = nullptr;
		for (VulkanModel* model : myPandaModels)
			delete model;
		myPandaModels.clear();
		delete myglTFModel;
		myglTFModel = nullptr;

		VulkanPSOContainer::DestroyDescriptorSetLayouts();

		myEmptyTexture.Destroy();

		ourInstance = nullptr;

		delete myDevice;

		if (locEnableValidationLayers)
			DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void VulkanRenderer::CreateVkInstance()
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

	void VulkanRenderer::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		FillDebugMessengerCreateInfo(createInfo);

		VK_CHECK_RESULT(CreateDebugMessenger(myVkInstance, &createInfo, nullptr, &myDebugMessenger), "Couldn't create a debug messenger!");
	}

	void VulkanRenderer::CreateDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, nullptr);
		if (deviceCount == 0)
			throw std::runtime_error("No physical device supporting Vulkan was found!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, devices.data());

		// TODO: Select the physical device based on requirements.
		// For now, just take the first one.
		myDevice = new VulkanDevice(devices[0]);

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

	void VulkanRenderer::SetupEmptyTexture()
	{
		myEmptyTexture.Create(1, 1,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		myEmptyTexture.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VulkanRenderer::GetInstance()->GetGraphicsQueue());

		VulkanBuffer textureStaging;
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
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, myEmptyTexture.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		EndOneTimeCommand(commandBuffer, VulkanRenderer::GetInstance()->GetGraphicsQueue());

		textureStaging.Destroy();

		myEmptyTexture.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VulkanRenderer::GetInstance()->GetGraphicsQueue());
		myEmptyTexture.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		myEmptyTexture.CreateImageSampler();
		myEmptyTexture.SetupDescriptor();
	}
}
