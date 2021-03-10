#include "VulkanRenderer.h"

#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

#include "VulkanCamera.h"
#include "VulkanModel.h"

namespace Render
{
	namespace
	{
		uint32_t locVulkanApiVersion = VK_API_VERSION_1_2;

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

		VulkanCamera::SetupDescriptorSetLayout();
		myCamera = new VulkanCamera();
		VulkanModel::SetupDescriptorSetLayout();
		myPandaModel = new VulkanModel();
	}

	VulkanRenderer::~VulkanRenderer()
	{
		delete myCamera;
		myCamera = nullptr;
		VulkanCamera::DestroyDescriptorSetLayout();
		delete myPandaModel;
		myPandaModel = nullptr;
		VulkanModel::DestroyDescriptorSetLayout();

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

}
