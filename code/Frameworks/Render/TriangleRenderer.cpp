#include "TriangleRenderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <assert.h>
#include <iostream>
#include <set>
#include <fstream>
#include <cstring>

namespace Render
{
	const uint32_t locWindowWidth = 800;
	const uint32_t locWindowHeight = 600;

	const std::vector<const char*> locValidationLayers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" };
#ifdef _DEBUG
	constexpr bool locEnableValidationLayers = true;
#else
	constexpr bool locEnableValidationLayers = false;
#endif
	const std::vector<const char*> locDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	const uint32_t locMaxFramesInFlight = 2;

	VkResult locCreateDebugUtilsMessengerEXT(
		VkInstance anInstance,
		const VkDebugUtilsMessengerCreateInfoEXT* aCreateInfo,
		const VkAllocationCallbacks* anAllocator,
		VkDebugUtilsMessengerEXT* aCallback)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(anInstance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(anInstance, aCreateInfo, anAllocator, aCallback);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void locDestroyDebugUtilsMessengerEXT(
		VkInstance anInstance,
		VkDebugUtilsMessengerEXT aCallback,
		const VkAllocationCallbacks* anAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(anInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(anInstance, aCallback, anAllocator);
		}
	}

	std::vector<char> locReadFile(const std::string& aFilename)
	{
		std::ifstream file(aFilename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error(std::string{ "Failed to read the file " } + aFilename + "!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	Render::TriangleRenderer::TriangleRenderer()
	{
		InitWindow();
		InitVulkan();
	}

	Render::TriangleRenderer::~TriangleRenderer()
	{
		Cleanup();
	}

	void Render::TriangleRenderer::Run()
	{
		while (!glfwWindowShouldClose(myWindow))
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(myLogicalDevice);
	}

	void Render::TriangleRenderer::InitWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		myWindow = glfwCreateWindow(locWindowWidth, locWindowHeight, "Vulkan Triangle Tuto", nullptr, nullptr);

		glfwSetWindowUserPointer(myWindow, this);
		glfwSetFramebufferSizeCallback(myWindow, FramebufferResizeCallback);
	}

	void Render::TriangleRenderer::InitVulkan()
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateSwapChainImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFrameBuffers();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSynchronizationObjects();
	}

	void TriangleRenderer::DrawFrame()
	{
		vkWaitForFences(myLogicalDevice, 1, &myInFlightFrameFences[myCurrentInFlightFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(myLogicalDevice, mySwapChain, UINT64_MAX, myImageAvailableSemaphores[myCurrentInFlightFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		if (myImageFences[imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(myLogicalDevice, 1, &myImageFences[imageIndex], VK_TRUE, UINT64_MAX);
		}

		myImageFences[imageIndex] = myInFlightFrameFences[myCurrentInFlightFrame];

		VkSemaphore waitSemaphores[] = { myImageAvailableSemaphores[myCurrentInFlightFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signalSemaphores[] = { myRenderFinishedSemaphores[myCurrentInFlightFrame] };

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(myLogicalDevice, 1, &myInFlightFrameFences[myCurrentInFlightFrame]);

		if (vkQueueSubmit(myGraphicsQueue, 1, &submitInfo, myInFlightFrameFences[myCurrentInFlightFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit a command buffer!");
		}

		VkSwapchainKHR swapChains[] = { mySwapChain };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(myPresentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || myFramebufferResized)
		{
			myFramebufferResized = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		myCurrentInFlightFrame = (myCurrentInFlightFrame + 1) % locMaxFramesInFlight;
	}

	void Render::TriangleRenderer::Cleanup()
	{
		for (uint32_t i = 0; i < locMaxFramesInFlight; ++i)
		{
			vkDestroyFence(myLogicalDevice, myInFlightFrameFences[i], nullptr);
			vkDestroySemaphore(myLogicalDevice, myRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(myLogicalDevice, myImageAvailableSemaphores[i], nullptr);
		}

		CleanupSwapChain();

		vkDestroyCommandPool(myLogicalDevice, myCommandPool, nullptr);

		vkDestroyDevice(myLogicalDevice, nullptr);

		vkDestroySurfaceKHR(myInstance, mySurface, nullptr);

		if (locEnableValidationLayers)
		{
			locDestroyDebugUtilsMessengerEXT(myInstance, myDebugCallback, nullptr);
		}

		vkDestroyInstance(myInstance, nullptr);

		glfwDestroyWindow(myWindow);

		glfwTerminate();
	}

	bool TriangleRenderer::CheckValidationLayersSupport(const std::vector<const char*>& someValidationLayers)
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (uint32_t i = 0, e = static_cast<uint32_t>(someValidationLayers.size()); i < e; ++i)
		{
			bool supported = false;
			for (uint32_t j = 0; j < layerCount; ++j)
			{
				if (strcmp(someValidationLayers[i], availableLayers[j].layerName) == 0)
				{
					supported = true;
					break;
				}
			}
			if (!supported)
			{
				return false;
			}
		}

		return true;
	}

	bool TriangleRenderer::CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions)
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		for (uint32_t i = 0, e = static_cast<uint32_t>(someExtensions.size()); i < e; ++i)
		{
			bool available = false;
			for (uint32_t j = 0; j < extensionCount; ++j)
			{
				if (strcmp(someExtensions[i], availableExtensions[j].extensionName) == 0)
				{
					available = true;
					break;
				}
			}
			if (!available)
			{
				return false;
			}
		}

		return true;
	}

	bool TriangleRenderer::CheckDeviceExtensionsSupport(VkPhysicalDevice aPhysicalDevice, const std::vector<const char*>& someExtensions)
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(aPhysicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(aPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

		for (uint32_t i = 0, e = static_cast<uint32_t>(someExtensions.size()); i < e; ++i)
		{
			bool available = false;
			for (uint32_t j = 0; j < extensionCount; ++j)
			{
				if (strcmp(someExtensions[i], availableExtensions[j].extensionName) == 0)
				{
					available = true;
					break;
				}
			}
			if (!available)
			{
				return false;
			}
		}

		return true;
	}

	bool TriangleRenderer::IsDeviceSuitable(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(aPhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(aPhysicalDevice, &deviceFeatures);
		// This cam help to find the best possible GPU to use based on the application needs
		(void)deviceProperties;
		(void)deviceFeatures;

		// Request that the device supports the extensions we need
		if (!CheckDeviceExtensionsSupport(aPhysicalDevice, locDeviceExtensions))
			return false;

		// Request that the device supports adequate SwapChain capabilities
		SwapChainSupportDetails swapChainSupportDetails;
		QuerySwapChainSupport(aPhysicalDevice, aSurface, swapChainSupportDetails);
		if (swapChainSupportDetails.myFormats.empty() || swapChainSupportDetails.myPresentModes.empty())
			return false;

		// Request that the device supports the queues we need
		QueueFamilyIndices queueIndices;
		FindQueueFamilies(aPhysicalDevice, aSurface, queueIndices);
		if (!queueIndices.IsComplete())
			return false;

		return true;
	}

	void TriangleRenderer::QuerySwapChainSupport(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface, SwapChainSupportDetails& someDetails)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(aPhysicalDevice, aSurface, &someDetails.myCapabilities);
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(aPhysicalDevice, aSurface, &formatCount, nullptr);
		someDetails.myFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(aPhysicalDevice, aSurface, &formatCount, someDetails.myFormats.data());

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(aPhysicalDevice, aSurface, &presentModeCount, nullptr);
		someDetails.myPresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(aPhysicalDevice, aSurface, &presentModeCount, someDetails.myPresentModes.data());
	}

	void TriangleRenderer::FindQueueFamilies(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface, QueueFamilyIndices& someQueueIndices)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(aPhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(aPhysicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				someQueueIndices.myGraphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(aPhysicalDevice, i, aSurface, &presentSupport);
			if (presentSupport)
			{
				someQueueIndices.myPresentFamily = i;
			}

			if (someQueueIndices.IsComplete())
			{
				break;
			}

			i++;
		}
	}

	VkExtent2D TriangleRenderer::SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR& someCapabilities)
	{
		if (someCapabilities.currentExtent.width != UINT32_MAX)
		{
			return someCapabilities.currentExtent;
		}
		else
		{
			VkExtent2D extent = { locWindowWidth, locWindowHeight };
			extent.width = std::max(someCapabilities.minImageExtent.width, std::min(someCapabilities.maxImageExtent.width, extent.width));
			extent.height = std::max(someCapabilities.minImageExtent.height, std::min(someCapabilities.maxImageExtent.height, extent.height));
			return extent;
		}
	}

	VkSurfaceFormatKHR TriangleRenderer::SelectSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& someAvailableFormats)
	{
		assert(someAvailableFormats.size() > 0);
		for (const auto& availableFormat : someAvailableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}
		return someAvailableFormats[0];
	}

	VkPresentModeKHR TriangleRenderer::SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR>& someAvailablePresentModes)
	{
		assert(someAvailablePresentModes.size() > 0);
		for (const auto& availablePresentMode : someAvailablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkShaderModule TriangleRenderer::CreateShaderModule(VkDevice aLogicalDevice, const std::vector<char>& someByteCode)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.codeSize = someByteCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(someByteCode.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(aLogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a module shader!");
		}
		return shaderModule;
	}

	void TriangleRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& aCreateInfo)
	{
		aCreateInfo = {};
		aCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		aCreateInfo.pNext = nullptr;
		aCreateInfo.flags = 0;
		aCreateInfo.messageSeverity =
//			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
//			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		aCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		aCreateInfo.pfnUserCallback = DebugCallback;
		aCreateInfo.pUserData = nullptr;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL TriangleRenderer::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT /*aMessageSeverity*/,
		VkDebugUtilsMessageTypeFlagsEXT /*aMessageType*/,
		const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
		void* /*aUserData*/)
	{
		std::cerr << "validation layer: " << aCallbackData->pMessage << std::endl;
		return VK_FALSE; // Don't interrupt the execution
	}

	void TriangleRenderer::FramebufferResizeCallback(GLFWwindow* aWindow, int /*aWidth*/, int /*aHeight*/)
	{
		auto app = reinterpret_cast<TriangleRenderer*>(glfwGetWindowUserPointer(aWindow));
		app->myFramebufferResized = true;
	}

	void TriangleRenderer::CreateInstance()
	{
		if (locEnableValidationLayers && !CheckValidationLayersSupport(locValidationLayers))
		{
			throw std::runtime_error("Validation layers are enabled but not supported!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Sora";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (!CheckInstanceExtensionsSupport(extensions))
		{
			throw std::runtime_error("Required extensions are not available!");
		}

		if (locEnableValidationLayers)
		{
			// Optional extension
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;
		if (locEnableValidationLayers)
		{
			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
			createInfo.enabledLayerCount = static_cast<uint32_t>(locValidationLayers.size());
			createInfo.ppEnabledLayerNames = locValidationLayers.data();
		}
		else
		{
			createInfo.pNext = nullptr;
			createInfo.enabledLayerCount = 0;
			//createInfo.ppEnabledLayerNames;
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &myInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance!");
		}
	}

	void TriangleRenderer::SetupDebugMessenger()
	{
		if (!locEnableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (locCreateDebugUtilsMessengerEXT(myInstance, &createInfo, nullptr, &myDebugCallback) != VK_SUCCESS)
		{
			throw std::runtime_error("Couldn't create a debug messenger!");
		}
	}

	void TriangleRenderer::CreateSurface()
	{
		if (glfwCreateWindowSurface(myInstance, myWindow, nullptr, &mySurface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the surface!");
		}
	}

	void TriangleRenderer::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(myInstance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("No physical device supporting Vulkan was found!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(myInstance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device, mySurface))
			{
				myPhysicalDevice = device;
				break;
			}
		}

		if (myPhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("No suitable physical device was found!");
		}
	}

	void TriangleRenderer::CreateLogicalDevice()
	{
		QueueFamilyIndices queueIndices;
		FindQueueFamilies(myPhysicalDevice, mySurface, queueIndices);
		assert(queueIndices.IsComplete());

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> queueFamilies = { queueIndices.myGraphicsFamily.value(), queueIndices.myPresentFamily.value() };
		for (uint32_t queueFamily : queueFamilies)
		{
			float queuePriority = 1.0f;

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		if (locEnableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(locValidationLayers.size());
			createInfo.ppEnabledLayerNames = locValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			//createInfo.ppEnabledLayerNames;
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(locDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = locDeviceExtensions.data();
		createInfo.pEnabledFeatures = &deviceFeatures;

		if (vkCreateDevice(myPhysicalDevice, &createInfo, nullptr, &myLogicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not create the logical device!");
		}

		vkGetDeviceQueue(myLogicalDevice, queueIndices.myGraphicsFamily.value(), 0, &myGraphicsQueue);
		vkGetDeviceQueue(myLogicalDevice, queueIndices.myPresentFamily.value(), 0, &myPresentQueue);
	}

	void TriangleRenderer::CreateSwapChain()
	{
		SwapChainSupportDetails swapChainSupportDetails;
		QuerySwapChainSupport(myPhysicalDevice, mySurface, swapChainSupportDetails);

		VkExtent2D extent = SelectSwapChainExtent(swapChainSupportDetails.myCapabilities);
		VkSurfaceFormatKHR surfaceFormat = SelectSwapChainFormat(swapChainSupportDetails.myFormats);
		VkPresentModeKHR presentMode = SelectSwapChainPresentMode(swapChainSupportDetails.myPresentModes);

		uint32_t imageCount = swapChainSupportDetails.myCapabilities.minImageCount + 1;
		if (swapChainSupportDetails.myCapabilities.maxImageCount > 0)
		{
			imageCount = std::min(imageCount, swapChainSupportDetails.myCapabilities.maxImageCount);
		}

		QueueFamilyIndices queueIndices;
		FindQueueFamilies(myPhysicalDevice, mySurface, queueIndices);
		uint32_t queueFamilyIndices[] = { queueIndices.myGraphicsFamily.value(), queueIndices.myPresentFamily.value() };

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = mySurface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (queueIndices.myGraphicsFamily != queueIndices.myPresentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		createInfo.preTransform = swapChainSupportDetails.myCapabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(myLogicalDevice, &createInfo, nullptr, &mySwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the swap chain!");
		}

		vkGetSwapchainImagesKHR(myLogicalDevice, mySwapChain, &imageCount, nullptr);
		mySwapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(myLogicalDevice, mySwapChain, &imageCount, mySwapChainImages.data());
		mySwapChainFormat = surfaceFormat.format;
		mySwapChainExtent = extent;
	}

	void TriangleRenderer::RecreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(myWindow, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwWaitEvents();
			glfwGetFramebufferSize(myWindow, &width, &height);
		}

		vkDeviceWaitIdle(myLogicalDevice);

		CleanupSwapChain();

		CreateSwapChain();
		CreateSwapChainImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFrameBuffers();
		CreateCommandBuffers();
	}

	void TriangleRenderer::CleanupSwapChain()
	{
		vkFreeCommandBuffers(myLogicalDevice, myCommandPool, static_cast<uint32_t>(myCommandBuffers.size()), myCommandBuffers.data());

		for (auto framebuffer : mySwapChainFramebuffers)
		{
			vkDestroyFramebuffer(myLogicalDevice, framebuffer, nullptr);
		}

		vkDestroyPipeline(myLogicalDevice, myGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(myLogicalDevice, myPipelineLayout, nullptr);

		vkDestroyRenderPass(myLogicalDevice, myRenderPass, nullptr);

		for (auto imageView : mySwapChainImageViews)
		{
			vkDestroyImageView(myLogicalDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(myLogicalDevice, mySwapChain, nullptr);
	}

	void TriangleRenderer::CreateSwapChainImageViews()
	{
		mySwapChainImageViews.resize(mySwapChainImages.size());
		for (size_t i = 0, e = mySwapChainImages.size(); i < e; ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.image = mySwapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = mySwapChainFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(myLogicalDevice, &createInfo, nullptr, &mySwapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create an image view for the swap chain!");
			}
		}
	}

	void TriangleRenderer::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.flags = 0;
		colorAttachment.format = mySwapChainFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = nullptr;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = 0;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(myLogicalDevice, &renderPassInfo, nullptr, &myRenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the render pass!");
		}
	}

	void TriangleRenderer::CreateGraphicsPipeline()
	{
		auto vertShaderCode = locReadFile("Frameworks/shaders/vert.spv");
		auto vertShaderModule = CreateShaderModule(myLogicalDevice, vertShaderCode);
		auto fragShaderCode = locReadFile("Frameworks/shaders/frag.spv");
		auto fragShaderModule = CreateShaderModule(myLogicalDevice, fragShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.pNext = nullptr;
		vertShaderStageInfo.flags = 0;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.pNext = nullptr;
		fragShaderStageInfo.flags = 0;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";
		fragShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.pNext = nullptr;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.vertexBindingDescriptionCount = 0;
		vertexInputStateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputStateInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
		inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateInfo.pNext = nullptr;
		inputAssemblyStateInfo.flags = 0;
		inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		inputAssemblyStateInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mySwapChainExtent.width);
		viewport.height = static_cast<float>(mySwapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = mySwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportStateInfo{};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.pNext = nullptr;
		viewportStateInfo.flags = 0;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.pViewports = &viewport;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizerStateInfo{};
		rasterizerStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerStateInfo.pNext = nullptr;
		rasterizerStateInfo.flags = 0;
		rasterizerStateInfo.depthClampEnable = VK_FALSE;
		rasterizerStateInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizerStateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerStateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerStateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizerStateInfo.depthBiasEnable = VK_FALSE;
		rasterizerStateInfo.depthBiasConstantFactor = 0.0f;
		rasterizerStateInfo.depthBiasClamp = 0.0f;
		rasterizerStateInfo.depthBiasSlopeFactor = 0.0f;
		rasterizerStateInfo.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisamplingStateInfo{};
		multisamplingStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingStateInfo.pNext = nullptr;
		multisamplingStateInfo.flags = 0;
		multisamplingStateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingStateInfo.sampleShadingEnable = VK_FALSE;
		multisamplingStateInfo.minSampleShading = 1.0f;
		multisamplingStateInfo.pSampleMask = nullptr;
		multisamplingStateInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingStateInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlendingStateInfo{};
		colorBlendingStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingStateInfo.pNext = nullptr;
		colorBlendingStateInfo.flags = 0;
		colorBlendingStateInfo.logicOpEnable = VK_FALSE;
		colorBlendingStateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendingStateInfo.attachmentCount = 1;
		colorBlendingStateInfo.pAttachments = &colorBlendAttachment;
		colorBlendingStateInfo.blendConstants[0] = 0.0f;
		colorBlendingStateInfo.blendConstants[1] = 0.0f;
		colorBlendingStateInfo.blendConstants[2] = 0.0f;
		colorBlendingStateInfo.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.pNext = nullptr;
		pipelineLayoutInfo.flags = 0;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(myLogicalDevice, &pipelineLayoutInfo, nullptr, &myPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;
		pipelineInfo.flags = 0;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputStateInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyStateInfo;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterizerStateInfo;
		pipelineInfo.pMultisampleState = &multisamplingStateInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendingStateInfo;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = myPipelineLayout;
		pipelineInfo.renderPass = myRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(myLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &myGraphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the graphic pipeline!");
		}

		vkDestroyShaderModule(myLogicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(myLogicalDevice, vertShaderModule, nullptr);
	}

	void TriangleRenderer::CreateFrameBuffers()
	{
		mySwapChainFramebuffers.resize(mySwapChainImageViews.size());

		for (size_t i = 0, e = mySwapChainImageViews.size(); i < e; ++i)
		{
			VkImageView attachments[] = { mySwapChainImageViews[i] };
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.pNext = nullptr;
			framebufferInfo.flags = 0;
			framebufferInfo.renderPass = myRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = mySwapChainExtent.width;
			framebufferInfo.height = mySwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(myLogicalDevice, &framebufferInfo, nullptr, &mySwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create a framebuffer!");
			}
		}
	}

	void TriangleRenderer::CreateCommandPool()
	{
		QueueFamilyIndices queueIndices;
		FindQueueFamilies(myPhysicalDevice, mySurface, queueIndices);

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = 0;
		commandPoolInfo.queueFamilyIndex = queueIndices.myGraphicsFamily.value();

		if (vkCreateCommandPool(myLogicalDevice, &commandPoolInfo, nullptr, &myCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the command pool");
		}
	}

	void TriangleRenderer::CreateCommandBuffers()
	{
		myCommandBuffers.resize(mySwapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = myCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(myCommandBuffers.size());

		if (vkAllocateCommandBuffers(myLogicalDevice, &allocInfo, myCommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command buffers!");
		}

		for (size_t i = 0, e = mySwapChainFramebuffers.size(); i < e; ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.flags = 0;
			beginInfo.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(myCommandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to begin a command buffer!");
			}

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.pNext = nullptr;
			renderPassInfo.renderPass = myRenderPass;
			renderPassInfo.framebuffer = mySwapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mySwapChainExtent;
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(myCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myGraphicsPipeline);

			vkCmdDraw(myCommandBuffers[i], 8, 1, 0, 0);

			vkCmdEndRenderPass(myCommandBuffers[i]);

			if (vkEndCommandBuffer(myCommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to end a command buffer!");
			}
		}
	}

	void TriangleRenderer::CreateSynchronizationObjects()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext = nullptr;
		semaphoreInfo.flags = 0;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext = nullptr;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		myImageAvailableSemaphores.resize(locMaxFramesInFlight, VK_NULL_HANDLE);
		myRenderFinishedSemaphores.resize(locMaxFramesInFlight, VK_NULL_HANDLE);
		myInFlightFrameFences.resize(locMaxFramesInFlight, VK_NULL_HANDLE);

		for (uint32_t i = 0; i < locMaxFramesInFlight; ++i)
		{
			if (vkCreateSemaphore(myLogicalDevice, &semaphoreInfo, nullptr, &myImageAvailableSemaphores[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create a semaphore!");
			}
			if (vkCreateSemaphore(myLogicalDevice, &semaphoreInfo, nullptr, &myRenderFinishedSemaphores[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create a semaphore!");
			}
			if (vkCreateFence(myLogicalDevice, &fenceInfo, nullptr, &myInFlightFrameFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create a fence!");
			}
		}

		myImageFences.resize(mySwapChainImages.size(), VK_NULL_HANDLE);
	}
}
