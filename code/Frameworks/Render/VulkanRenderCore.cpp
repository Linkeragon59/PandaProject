#include "VulkanRenderCore.h"

#include "VulkanHelpers.h"
#include "VulkanDebugMessenger.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <assert.h>
#include <set>

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
	
	VulkanRenderCore* VulkanRenderCore::ourInstance = nullptr;

	void VulkanRenderCore::CreateInstance()
	{
		assert(!ourInstance);
		
		glfwInit();
		
		ourInstance = new VulkanRenderCore();
	}

	void VulkanRenderCore::DestroyInstance()
	{
		assert(ourInstance);

		delete ourInstance;
		ourInstance = nullptr;

		glfwTerminate();
	}

	void VulkanRenderCore::Update()
	{
		for (uint32_t i = 0; i < (uint32_t)mySwapChains.size(); ++i)
		{
			mySwapChains[i]->Update();
		}
	}

	GLFWwindow* VulkanRenderCore::OpenWindow(int aWidth, int aHeight, const char* aTitle)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		GLFWwindow* window = glfwCreateWindow(aWidth, aHeight, aTitle, nullptr, nullptr);

		LoadDummyScene();

		VulkanSwapChain* swapChain = new VulkanSwapChain(window);
		mySwapChains.push_back(swapChain);

		return window;
	}

	void VulkanRenderCore::CloseWindow(GLFWwindow* aWindow)
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

		UnloadDummyScene();

		glfwDestroyWindow(aWindow);
	}

	VkPhysicalDevice VulkanRenderCore::GetPhysicalDevice() const
	{
		return myDevice->myPhysicalDevice;
	}

	VkDevice VulkanRenderCore::GetDevice() const
	{
		return myDevice->myLogicalDevice;
	}

	VmaAllocator VulkanRenderCore::GetAllocator() const
	{
		return myDevice->myVmaAllocator;
	}

	VkQueue VulkanRenderCore::GetGraphicsQueue() const
	{
		return myDevice->myGraphicsQueue;
	}

	VkCommandPool VulkanRenderCore::GetGraphicsCommandPool() const
	{
		return myDevice->myGraphicsCommandPool;
	}

	VulkanRenderCore::VulkanRenderCore()
	{
		CreateVkInstance();

		if (locEnableValidationLayers)
			SetupDebugMessenger();

		CreateDevice();
	}

	VulkanRenderCore::~VulkanRenderCore()
	{		
		delete myDevice;

		if (locEnableValidationLayers)
			DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void VulkanRenderCore::CreateVkInstance()
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

	void VulkanRenderCore::SetupDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		FillDebugMessengerCreateInfo(createInfo);

		VK_CHECK_RESULT(CreateDebugMessenger(myVkInstance, &createInfo, nullptr, &myDebugMessenger), "Couldn't create a debug messenger!");
	}

	void VulkanRenderCore::CreateDevice()
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

	void VulkanRenderCore::LoadDummyScene()
	{
		myScene = new vkglTF::Model();

		const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
		myScene->loadFromFile("Frameworks/models/treasure_smooth.gltf", myDevice, myDevice->myGraphicsQueue, glTFLoadingFlags);
	}

	void VulkanRenderCore::UnloadDummyScene()
	{
		delete myScene;
		myScene = nullptr;
	}

}

