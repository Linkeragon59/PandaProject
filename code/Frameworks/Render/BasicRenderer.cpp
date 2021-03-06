#include "BasicRenderer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#include <stb_image.h>
#ifdef __linux__
#pragma GCC diagnostic pop
#endif

#if USE_VMA
#if defined(_WINDOWS)
#pragma warning(push)
#pragma warning(disable:4100)
#pragma warning(disable:4127)
#pragma warning(disable:4324)
#elif defined(__linux__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#endif
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#if defined(_WINDOWS)
#pragma warning(pop)
#elif defined(__linux__)
#pragma GCC diagnostic pop
#endif

#include <stdexcept>
#include <assert.h>
#include <iostream>
#include <set>
#include <fstream>
#include <cstring>
#include <array>
#include <chrono>
#include <thread>

#include "Input.h"

namespace Render
{
	namespace
	{
		const uint32_t locWindowWidth = 800;
		const uint32_t locWindowHeight = 600;

		const std::vector<const char*> locValidationLayers = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor" };
#if defined(_WINDOWS) && !defined(NDEBUG)
		constexpr bool locEnableValidationLayers = true;
#else
		constexpr bool locEnableValidationLayers = false;
#endif
		const std::vector<const char*> locDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		const uint32_t locMaxFramesInFlight = 2;

		struct Vertex
		{
			glm::vec3 myPos;
			glm::vec3 myColor;
			glm::vec2 myTexCoord;

			static VkVertexInputBindingDescription GetBindingDescription()
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, myPos);
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, myColor);
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[2].offset = offsetof(Vertex, myTexCoord);
				return attributeDescriptions;
			}
		};

		struct UniformBufferObject
		{
			glm::mat4 myModel;
			glm::mat4 myView;
			glm::mat4 myProj;
		};

		const std::string locTestModelTexture = "Frameworks/textures/viking_room.png";
		const std::string locTestModel = "Frameworks/models/viking_room.obj";

		const std::string locTestTexture = "Frameworks/textures/panda.jpg";

		const std::vector<Vertex> locVertices =
		{
			{{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}},
			{{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.0f}},
			{{0.87f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.065f, 0.25f}},
			{{0.87f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.065f, 0.75f}},
			{{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.5f, 1.0f}},
			{{-0.87f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.935f, 0.75f}},
			{{-0.87f, -0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.935f, 0.25f}},

			{{0.0f, 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f}},
			{{0.0f, -1.0f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.0f}},
			{{0.87f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {0.065f, 0.25f}},
			{{0.87f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.065f, 0.75f}},
			{{0.0f, 1.0f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.5f, 1.0f}},
			{{-0.87f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.935f, 0.75f}},
			{{-0.87f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.935f, 0.25f}}
		};

		const std::vector<uint16_t> locIndices =
		{
			0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 1,

			7, 8, 9, 7, 9, 10, 7, 10, 11, 7, 11, 12, 7, 12, 13, 7, 13, 8
		};

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
	}

	BasicRenderer::BasicRenderer(GLFWwindow* aWindow)
	{
		myWindow = aWindow;

		glfwSetWindowUserPointer(myWindow, this);
		glfwSetFramebufferSizeCallback(myWindow, FramebufferResizeCallback);

		InitVulkan();
	}

	BasicRenderer::~BasicRenderer()
	{
		vkDeviceWaitIdle(myLogicalDevice);
		Cleanup();
	}

	void BasicRenderer::Update()
	{
		DrawFrame();
	}

	void BasicRenderer::InitVulkan()
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
#if USE_VMA
		CreateVmaAllocator();
#endif
		CreateSwapChain();
		CreateSwapChainImageViews();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateCommandPool();
		CreateDepthResources();
		CreateFrameBuffers();
		CreateTextureImage();
		CreateTextureImageView();
		CreateTextureSampler();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
		CreateSynchronizationObjects();
	}

	void BasicRenderer::DrawFrame()
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

		UpdateUniformBuffers(imageIndex);

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

	void BasicRenderer::Cleanup()
	{
		for (uint32_t i = 0; i < locMaxFramesInFlight; ++i)
		{
			vkDestroyFence(myLogicalDevice, myInFlightFrameFences[i], nullptr);
			vkDestroySemaphore(myLogicalDevice, myRenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(myLogicalDevice, myImageAvailableSemaphores[i], nullptr);
		}

		CleanupSwapChain();

		vkDestroyDescriptorSetLayout(myLogicalDevice, myDescriptorSetLayout, nullptr);

#if USE_VMA
		vmaDestroyBuffer(myVmaAllocator, myIndexBuffer, myIndexBufferAlloc);
		vmaDestroyBuffer(myVmaAllocator, myVertexBuffer, myVertexBufferAlloc);
#else
		vkDestroyBuffer(myLogicalDevice, myIndexBuffer, nullptr);
		vkFreeMemory(myLogicalDevice, myIndexBufferMemory, nullptr);
		vkDestroyBuffer(myLogicalDevice, myVertexBuffer, nullptr);
		vkFreeMemory(myLogicalDevice, myVertexBufferMemory, nullptr);
#endif

		vkDestroySampler(myLogicalDevice, myTextureSampler, nullptr);

		vkDestroyImageView(myLogicalDevice, myTextureImageView, nullptr);
#if USE_VMA
		vmaDestroyImage(myVmaAllocator, myTextureImage, myTextureImageAlloc);
#else
		vkDestroyImage(myLogicalDevice, myTextureImage, nullptr);
		vkFreeMemory(myLogicalDevice, myTextureImageMemory, nullptr);
#endif

		vkDestroyCommandPool(myLogicalDevice, myCommandPool, nullptr);

#if USE_VMA
		vmaDestroyAllocator(myVmaAllocator);
#endif

		vkDestroyDevice(myLogicalDevice, nullptr);

		vkDestroySurfaceKHR(myInstance, mySurface, nullptr);

		if (locEnableValidationLayers)
		{
			locDestroyDebugUtilsMessengerEXT(myInstance, myDebugCallback, nullptr);
		}

		vkDestroyInstance(myInstance, nullptr);
	}

	bool BasicRenderer::CheckValidationLayersSupport(const std::vector<const char*>& someValidationLayers)
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

	bool BasicRenderer::CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions)
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

	bool BasicRenderer::CheckDeviceExtensionsSupport(VkPhysicalDevice aPhysicalDevice, const std::vector<const char*>& someExtensions)
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

#if !USE_VMA
	uint32_t BasicRenderer::FindMemoryType(VkPhysicalDevice aPhysicalDevice, uint32_t aTypeFilter, VkMemoryPropertyFlags someProperties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(aPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
		{
			if (aTypeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & someProperties) == someProperties)
			{
				return i;
			}
		}

		throw std::runtime_error("Couldn't find a memory type satisfying the requirements");
	}
#endif

	bool BasicRenderer::IsDeviceSuitable(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(aPhysicalDevice, &deviceProperties);
		vkGetPhysicalDeviceFeatures(aPhysicalDevice, &deviceFeatures);
		// This can help to find the best possible GPU to use based on the application needs
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

	void BasicRenderer::QuerySwapChainSupport(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface, SwapChainSupportDetails& someOutDetails)
	{
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(aPhysicalDevice, aSurface, &someOutDetails.myCapabilities);
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(aPhysicalDevice, aSurface, &formatCount, nullptr);
		someOutDetails.myFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(aPhysicalDevice, aSurface, &formatCount, someOutDetails.myFormats.data());

		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(aPhysicalDevice, aSurface, &presentModeCount, nullptr);
		someOutDetails.myPresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(aPhysicalDevice, aSurface, &presentModeCount, someOutDetails.myPresentModes.data());
	}

	void BasicRenderer::FindQueueFamilies(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface, QueueFamilyIndices& someOutQueueIndices)
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
				someOutQueueIndices.myGraphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(aPhysicalDevice, i, aSurface, &presentSupport);
			if (presentSupport)
			{
				someOutQueueIndices.myPresentFamily = i;
			}

			if (someOutQueueIndices.IsComplete())
			{
				break;
			}

			i++;
		}
	}

	VkFormat BasicRenderer::FindSupportedFormat(VkPhysicalDevice aPhysicalDevice, const std::vector<VkFormat>& someCandidateFormats, VkImageTiling aTiling, VkFormatFeatureFlags someFeatures)
	{
		for (VkFormat format : someCandidateFormats)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(aPhysicalDevice, format, &props);
			
			if (aTiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & someFeatures) == someFeatures)
			{
				return format;
			}
			else if (aTiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & someFeatures) == someFeatures)
			{
				return format;
			}
		}

		throw std::runtime_error("Couldn't find an available format!");
	}

	VkFormat BasicRenderer::FindBestDepthFormat(VkPhysicalDevice aPhysicalDevice)
	{
		return FindSupportedFormat(
			aPhysicalDevice,
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
	}

	VkExtent2D BasicRenderer::SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR& someCapabilities)
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

	VkSurfaceFormatKHR BasicRenderer::SelectSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& someAvailableFormats)
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

	VkPresentModeKHR BasicRenderer::SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR>& someAvailablePresentModes)
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

	VkShaderModule BasicRenderer::CreateShaderModule(VkDevice aLogicalDevice, const std::vector<char>& someByteCode)
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

	VkCommandBuffer BasicRenderer::BeginOneShotCommand(VkDevice aLogicalDevice, VkCommandPool aCommandPool)
	{
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.commandPool = aCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(aLogicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a command buffer!");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin a command buffer!");
		}

		return commandBuffer;
	}

	void BasicRenderer::EndOneShotCommand(VkDevice aLogicalDevice, VkCommandPool aCommandPool, VkQueue aQueue, VkCommandBuffer aCommandBuffer)
	{
		if (vkEndCommandBuffer(aCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to end a command buffer!");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &aCommandBuffer;

		vkQueueSubmit(aQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(aQueue);

		vkFreeCommandBuffers(aLogicalDevice, aCommandPool, 1, &aCommandBuffer);
	}

	void BasicRenderer::TransitionImageLayout(VkDevice aLogicalDevice, VkCommandPool aCommandPool, VkQueue aQueue, VkImage anImage, VkFormat aFormat, VkImageLayout anOldLayout, VkImageLayout aNewLayout)
	{
		VkCommandBuffer commandBuffer = BeginOneShotCommand(aLogicalDevice, aCommandPool);

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		if (anOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && aNewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (anOldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && aNewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (anOldLayout == VK_IMAGE_LAYOUT_UNDEFINED && aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else
		{
			throw std::invalid_argument("Not supporting this image transition yet!");
		}
		barrier.oldLayout = anOldLayout;
		barrier.newLayout = aNewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = anImage;
		if (aNewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (aFormat == VK_FORMAT_D16_UNORM_S8_UINT || aFormat == VK_FORMAT_D24_UNORM_S8_UINT || aFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndOneShotCommand(aLogicalDevice, aCommandPool, aQueue, commandBuffer);
	}

#if USE_VMA
	void BasicRenderer::CreateImage(VmaAllocator aVmaAllocator, uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkImage& anOutImage, VmaAllocation& anOutImageAlloc)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.flags = 0;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = aFormat;
		imageInfo.extent.width = aWidth;
		imageInfo.extent.height = aHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = aTiling;
		imageInfo.usage = aUsage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0; // Ignored when using VK_SHARING_MODE_EXCLUSIVE
		imageInfo.pQueueFamilyIndices = nullptr;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.flags = 0; // Could pre-map the memory for staging buffers using VMA_ALLOCATION_CREATE_MAPPED_BIT
		if (someProperties == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else
		{
			throw std::runtime_error("Image allocation usage not supported yet.");
		}
		allocInfo.requiredFlags = someProperties;
		allocInfo.preferredFlags = 0;
		allocInfo.memoryTypeBits = 0;
		allocInfo.pool = nullptr;
		allocInfo.pUserData = nullptr;
		allocInfo.priority = 0.0f;

		if (vmaCreateImage(aVmaAllocator, &imageInfo, &allocInfo, &anOutImage, &anOutImageAlloc, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!");
		}
	}

	void BasicRenderer::CreateBuffer(VmaAllocator aVmaAllocator, VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkBuffer& anOutBuffer, VmaAllocation& anOutBufferAlloc)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.flags = 0;
		bufferInfo.size = aSize;
		bufferInfo.usage = aUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = nullptr;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.flags = 0;
		if (someProperties == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}
		else if (someProperties == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		{
			if (aUsage == VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
			{
				allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
			}
			else
			{
				allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			}
		}
		else
		{
			throw std::runtime_error("Buffer allocation usage not supported yet.");
		}
		allocInfo.requiredFlags = someProperties;
		allocInfo.preferredFlags = 0;
		allocInfo.memoryTypeBits = 0;
		allocInfo.pool = nullptr;
		allocInfo.pUserData = nullptr;
		allocInfo.priority = 0.0f;

		if (vmaCreateBuffer(aVmaAllocator, &bufferInfo, &allocInfo, &anOutBuffer, &anOutBufferAlloc, nullptr) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate buffer memory!");
		}
	}
#else
	void BasicRenderer::CreateImage(VkPhysicalDevice aPhysicalDevice, VkDevice aLogicalDevice, uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkImage& anOutImage, VkDeviceMemory& anOutImageMemory)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.flags = 0;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = aFormat;
		imageInfo.extent.width = aWidth;
		imageInfo.extent.height = aHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = aTiling;
		imageInfo.usage = aUsage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 0; // Ignored when using VK_SHARING_MODE_EXCLUSIVE
		imageInfo.pQueueFamilyIndices = nullptr;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		if (vkCreateImage(aLogicalDevice, &imageInfo, nullptr, &anOutImage) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create an image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(aLogicalDevice, anOutImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(aPhysicalDevice, memRequirements.memoryTypeBits, someProperties);

		if (vkAllocateMemory(aLogicalDevice, &allocInfo, nullptr, &anOutImageMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate image memory!");
		}

		vkBindImageMemory(aLogicalDevice, anOutImage, anOutImageMemory, 0);
	}

	void BasicRenderer::CreateBuffer(
		VkPhysicalDevice aPhysicalDevice,
		VkDevice aLogicalDevice,
		VkDeviceSize aSize,
		VkBufferUsageFlags aUsage,
		VkMemoryPropertyFlags someProperties,
		VkBuffer& anOutBuffer,
		VkDeviceMemory& anOutBufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.flags = 0;
		bufferInfo.size = aSize;
		bufferInfo.usage = aUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.queueFamilyIndexCount = 0;
		bufferInfo.pQueueFamilyIndices = nullptr;

		if (vkCreateBuffer(aLogicalDevice, &bufferInfo, nullptr, &anOutBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(aLogicalDevice, anOutBuffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(aPhysicalDevice, memRequirements.memoryTypeBits, someProperties);

		if (vkAllocateMemory(aLogicalDevice, &allocInfo, nullptr, &anOutBufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate memory!");
		}

		vkBindBufferMemory(aLogicalDevice, anOutBuffer, anOutBufferMemory, 0);
	}
#endif

	void BasicRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& aCreateInfo)
	{
		aCreateInfo = {};
		aCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		aCreateInfo.pNext = nullptr;
		aCreateInfo.flags = 0;
		aCreateInfo.messageSeverity =
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		aCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		aCreateInfo.pfnUserCallback = DebugCallback;
		aCreateInfo.pUserData = nullptr;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL BasicRenderer::DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT /*aMessageSeverity*/,
		VkDebugUtilsMessageTypeFlagsEXT /*aMessageType*/,
		const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
		void* /*aUserData*/)
	{
		std::cerr << "validation layer: " << aCallbackData->pMessage << std::endl;
		return VK_FALSE; // Don't interrupt the execution
	}

	void BasicRenderer::FramebufferResizeCallback(GLFWwindow* aWindow, int /*aWidth*/, int /*aHeight*/)
	{
		auto app = reinterpret_cast<BasicRenderer*>(glfwGetWindowUserPointer(aWindow));
		app->myFramebufferResized = true;
	}

	void BasicRenderer::CreateInstance()
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
			createInfo.ppEnabledLayerNames = nullptr;
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &myInstance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Vulkan instance!");
		}
	}

	void BasicRenderer::SetupDebugMessenger()
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

	void BasicRenderer::CreateSurface()
	{
		if (glfwCreateWindowSurface(myInstance, myWindow, nullptr, &mySurface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the surface!");
		}
	}

	void BasicRenderer::PickPhysicalDevice()
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
		
		vkGetPhysicalDeviceFeatures(myPhysicalDevice, &myAvailableFeatures);
	}

	void BasicRenderer::CreateLogicalDevice()
	{
		if (myAvailableFeatures.samplerAnisotropy == VK_TRUE)
			myRequestedFeatures.samplerAnisotropy = VK_TRUE;

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
			createInfo.ppEnabledLayerNames = nullptr;
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(locDeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = locDeviceExtensions.data();
		createInfo.pEnabledFeatures = &myRequestedFeatures;

		if (vkCreateDevice(myPhysicalDevice, &createInfo, nullptr, &myLogicalDevice) != VK_SUCCESS)
		{
			throw std::runtime_error("Could not create the logical device!");
		}

		vkGetDeviceQueue(myLogicalDevice, queueIndices.myGraphicsFamily.value(), 0, &myGraphicsQueue);
		vkGetDeviceQueue(myLogicalDevice, queueIndices.myPresentFamily.value(), 0, &myPresentQueue);
	}

#if USE_VMA
	void BasicRenderer::CreateVmaAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.flags = 0;
		allocatorInfo.physicalDevice = myPhysicalDevice;
		allocatorInfo.device = myLogicalDevice;
		allocatorInfo.preferredLargeHeapBlockSize = 0; // defaults to 256 MiB
		allocatorInfo.pAllocationCallbacks = nullptr;
		allocatorInfo.pDeviceMemoryCallbacks = nullptr;
		allocatorInfo.frameInUseCount = locMaxFramesInFlight;
		allocatorInfo.pHeapSizeLimit = nullptr;
		allocatorInfo.pVulkanFunctions = nullptr;
		allocatorInfo.pRecordSettings = nullptr;
		allocatorInfo.instance = myInstance;
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;

		if (vmaCreateAllocator(&allocatorInfo, &myVmaAllocator) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the vma allocator!");
		}
	}
#endif

	void BasicRenderer::CreateSwapChain()
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

	void BasicRenderer::RecreateSwapChain()
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
		CreateDepthResources();
		CreateFrameBuffers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();

		myImageFences.resize(mySwapChainImages.size(), VK_NULL_HANDLE);
	}

	void BasicRenderer::CleanupSwapChain()
	{
		vkFreeCommandBuffers(myLogicalDevice, myCommandPool, static_cast<uint32_t>(myCommandBuffers.size()), myCommandBuffers.data());

		for (auto framebuffer : mySwapChainFramebuffers)
		{
			vkDestroyFramebuffer(myLogicalDevice, framebuffer, nullptr);
		}

		for (size_t i = 0; i < mySwapChainImages.size(); ++i)
		{
#if USE_VMA
			vmaDestroyBuffer(myVmaAllocator, myUniformBuffers[i], myUniformBuffersAllocs[i]);
#else
			vkDestroyBuffer(myLogicalDevice, myUniformBuffers[i], nullptr);
			vkFreeMemory(myLogicalDevice, myUniformBuffersMemory[i], nullptr);
#endif
		}

		vkDestroyDescriptorPool(myLogicalDevice, myDescriptorPool, nullptr);

		vkDestroyPipeline(myLogicalDevice, myGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(myLogicalDevice, myPipelineLayout, nullptr);

		vkDestroyRenderPass(myLogicalDevice, myRenderPass, nullptr);

		vkDestroyImageView(myLogicalDevice, myDepthImageView, nullptr);
#if USE_VMA
		vmaDestroyImage(myVmaAllocator, myDepthImage, myDepthImageAlloc);
#else
		vkDestroyImage(myLogicalDevice, myDepthImage, nullptr);
		vkFreeMemory(myLogicalDevice, myDepthImageMemory, nullptr);
#endif

		for (auto imageView : mySwapChainImageViews)
		{
			vkDestroyImageView(myLogicalDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(myLogicalDevice, mySwapChain, nullptr);
	}

	void BasicRenderer::CreateSwapChainImageViews()
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

	void BasicRenderer::CreateRenderPass()
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

		VkAttachmentDescription depthAttachment{};
		depthAttachment.flags = 0;
		depthAttachment.format = FindBestDepthFormat(myPhysicalDevice);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.flags = 0;
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
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

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.flags = 0;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(myLogicalDevice, &renderPassInfo, nullptr, &myRenderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the render pass!");
		}
	}

	void BasicRenderer::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(myLogicalDevice, &layoutInfo, nullptr, &myDescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a descriptor set!");
		}
	}

	void BasicRenderer::CreateGraphicsPipeline()
	{
		auto vertShaderCode = locReadFile("Frameworks/shaders/basicRenderer_vert.spv");
		auto vertShaderModule = CreateShaderModule(myLogicalDevice, vertShaderCode);
		auto fragShaderCode = locReadFile("Frameworks/shaders/basicRenderer_frag.spv");
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

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{};
		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.pNext = nullptr;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputStateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo{};
		inputAssemblyStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateInfo.pNext = nullptr;
		inputAssemblyStateInfo.flags = 0;
		inputAssemblyStateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
		rasterizerStateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

		VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo{};
		depthStencilStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateInfo.pNext = nullptr;
		depthStencilStateInfo.flags = 0;
		depthStencilStateInfo.depthTestEnable = VK_TRUE;
		depthStencilStateInfo.depthWriteEnable = VK_TRUE;
		depthStencilStateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateInfo.stencilTestEnable = VK_FALSE;
		depthStencilStateInfo.front = {};
		depthStencilStateInfo.back = {};
		depthStencilStateInfo.minDepthBounds = 0.0f;
		depthStencilStateInfo.maxDepthBounds = 1.0f;

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
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &myDescriptorSetLayout;
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
		pipelineInfo.pDepthStencilState = &depthStencilStateInfo;
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

	void BasicRenderer::CreateFrameBuffers()
	{
		mySwapChainFramebuffers.resize(mySwapChainImageViews.size());

		for (size_t i = 0, e = mySwapChainImageViews.size(); i < e; ++i)
		{
			std::array<VkImageView, 2> attachments = { mySwapChainImageViews[i], myDepthImageView };

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.pNext = nullptr;
			framebufferInfo.flags = 0;
			framebufferInfo.renderPass = myRenderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = mySwapChainExtent.width;
			framebufferInfo.height = mySwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(myLogicalDevice, &framebufferInfo, nullptr, &mySwapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create a framebuffer!");
			}
		}
	}

	void BasicRenderer::CreateCommandPool()
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

	void BasicRenderer::CreateDepthResources()
	{
		VkFormat bestDepthFormat = FindBestDepthFormat(myPhysicalDevice);

#if USE_VMA
		CreateImage(myVmaAllocator, mySwapChainExtent.width, mySwapChainExtent.height, bestDepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, myDepthImage, myDepthImageAlloc);
#else
		CreateImage(myPhysicalDevice, myLogicalDevice, mySwapChainExtent.width, mySwapChainExtent.height, bestDepthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, myDepthImage, myDepthImageMemory);
#endif

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = myDepthImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = bestDepthFormat;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(myLogicalDevice, &viewInfo, nullptr, &myDepthImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a view for the depth image!");
		}

		// optional
		TransitionImageLayout(myLogicalDevice, myCommandPool, myGraphicsQueue, myDepthImage, bestDepthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}

	void BasicRenderer::CreateTextureImage()
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(locTestTexture.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (!pixels)
		{
			throw std::runtime_error("Failed to load an image!");
		}

		VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4;

		VkBuffer stagingBuffer;
#if USE_VMA
		VmaAllocation stagingBufferAlloc;
		CreateBuffer(myVmaAllocator, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferAlloc);

		void* data;
		vmaMapMemory(myVmaAllocator, stagingBufferAlloc, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vmaUnmapMemory(myVmaAllocator, stagingBufferAlloc);
#else
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(myPhysicalDevice, myLogicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(myLogicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(myLogicalDevice, stagingBufferMemory);
#endif

		stbi_image_free(pixels);

#if USE_VMA
		CreateImage(myVmaAllocator, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, myTextureImage, myTextureImageAlloc);
#else
		CreateImage(myPhysicalDevice, myLogicalDevice, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, myTextureImage, myTextureImageMemory);
#endif

		TransitionImageLayout(myLogicalDevice, myCommandPool, myGraphicsQueue, myTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkCommandBuffer commandBuffer = BeginOneShotCommand(myLogicalDevice, myCommandPool);

		VkBufferImageCopy copyRegion{};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;
		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageOffset = { 0, 0, 0 };
		copyRegion.imageExtent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };
		vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, myTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		EndOneShotCommand(myLogicalDevice, myCommandPool, myGraphicsQueue, commandBuffer);

		TransitionImageLayout(myLogicalDevice, myCommandPool, myGraphicsQueue, myTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

#if USE_VMA
		vmaDestroyBuffer(myVmaAllocator, stagingBuffer, stagingBufferAlloc);
#else
		vkDestroyBuffer(myLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(myLogicalDevice, stagingBufferMemory, nullptr);
#endif
	}

	void BasicRenderer::CreateTextureImageView()
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.flags = 0;
		viewInfo.image = myTextureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		if (vkCreateImageView(myLogicalDevice, &viewInfo, nullptr, &myTextureImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a view for the texture!");
		}
	}

	void BasicRenderer::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0.0f;
		if (myRequestedFeatures.samplerAnisotropy == VK_TRUE)
		{
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16.0f;
		}
		else
		{
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 1.0f;
		}
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		if (vkCreateSampler(myLogicalDevice, &samplerInfo, nullptr, &myTextureSampler) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a sampler!");
		}
	}

	void BasicRenderer::CreateVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(Vertex) * locVertices.size();

		VkBuffer stagingBuffer;
#if USE_VMA
		VmaAllocation stagingBufferAlloc;
		CreateBuffer(
			myVmaAllocator,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferAlloc);

		void* data;
		vmaMapMemory(myVmaAllocator, stagingBufferAlloc, &data);
		memcpy(data, locVertices.data(), (size_t)bufferSize);
		vmaUnmapMemory(myVmaAllocator, stagingBufferAlloc);

		CreateBuffer(
			myVmaAllocator,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			myVertexBuffer,
			myVertexBufferAlloc);
#else
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(
			myPhysicalDevice,
			myLogicalDevice,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(myLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, locVertices.data(), (size_t)bufferSize);
		vkUnmapMemory(myLogicalDevice, stagingBufferMemory);

		CreateBuffer(
			myPhysicalDevice,
			myLogicalDevice,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			myVertexBuffer,
			myVertexBufferMemory);
#endif

		VkCommandBuffer commandBuffer = BeginOneShotCommand(myLogicalDevice, myCommandPool);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(commandBuffer, stagingBuffer, myVertexBuffer, 1, &copyRegion);

		EndOneShotCommand(myLogicalDevice, myCommandPool, myGraphicsQueue, commandBuffer);

#if USE_VMA
		vmaDestroyBuffer(myVmaAllocator, stagingBuffer, stagingBufferAlloc);
#else
		vkDestroyBuffer(myLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(myLogicalDevice, stagingBufferMemory, nullptr);
#endif
	}

	void BasicRenderer::CreateIndexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(uint16_t) * locIndices.size();

		VkBuffer stagingBuffer;
#if USE_VMA
		VmaAllocation stagingBufferAlloc;
		CreateBuffer(
			myVmaAllocator,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferAlloc);

		void* data;
		vmaMapMemory(myVmaAllocator, stagingBufferAlloc, &data);
		memcpy(data, locIndices.data(), (size_t)bufferSize);
		vmaUnmapMemory(myVmaAllocator, stagingBufferAlloc);

		CreateBuffer(
			myVmaAllocator,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			myIndexBuffer,
			myIndexBufferAlloc);
#else
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(
			myPhysicalDevice,
			myLogicalDevice,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(myLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, locIndices.data(), (size_t)bufferSize);
		vkUnmapMemory(myLogicalDevice, stagingBufferMemory);

		CreateBuffer(
			myPhysicalDevice,
			myLogicalDevice,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			myIndexBuffer,
			myIndexBufferMemory);
#endif

		VkCommandBuffer commandBuffer = BeginOneShotCommand(myLogicalDevice, myCommandPool);

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = bufferSize;
		vkCmdCopyBuffer(commandBuffer, stagingBuffer, myIndexBuffer, 1, &copyRegion);

		EndOneShotCommand(myLogicalDevice, myCommandPool, myGraphicsQueue, commandBuffer);

#if USE_VMA
		vmaDestroyBuffer(myVmaAllocator, stagingBuffer, stagingBufferAlloc);
#else
		vkDestroyBuffer(myLogicalDevice, stagingBuffer, nullptr);
		vkFreeMemory(myLogicalDevice, stagingBufferMemory, nullptr);
#endif
	}

	void BasicRenderer::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		myUniformBuffers.resize(mySwapChainImages.size());
#if USE_VMA
		myUniformBuffersAllocs.resize(mySwapChainImages.size());
#else
		myUniformBuffersMemory.resize(mySwapChainImages.size());
#endif

		for (size_t i = 0; i < mySwapChainImages.size(); ++i)
		{
#if USE_VMA
			CreateBuffer(
				myVmaAllocator,
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				myUniformBuffers[i],
				myUniformBuffersAllocs[i]);
#else
			CreateBuffer(
				myPhysicalDevice,
				myLogicalDevice,
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				myUniformBuffers[i],
				myUniformBuffersMemory[i]);
#endif
		}
	}

	void BasicRenderer::CreateDescriptorPool()
	{
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(mySwapChainImages.size());
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(mySwapChainImages.size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.flags = 0;
		poolInfo.maxSets = static_cast<uint32_t>(mySwapChainImages.size());
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		if (vkCreateDescriptorPool(myLogicalDevice, &poolInfo, nullptr, &myDescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the descriptor pool!");
		}
	}

	void BasicRenderer::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(mySwapChainImages.size(), myDescriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = myDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		myDescriptorSets.resize(mySwapChainImages.size());

		if (vkAllocateDescriptorSets(myLogicalDevice, &allocInfo, myDescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate the descriptor sets!");
		}

		for (size_t i = 0; i < mySwapChainImages.size(); ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = myUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkDescriptorImageInfo imageInfo{};
			imageInfo.sampler = myTextureSampler;
			imageInfo.imageView = myTextureImageView;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].pNext = nullptr;
			descriptorWrites[0].dstSet = myDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].pImageInfo = nullptr;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pTexelBufferView = nullptr;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].pNext = nullptr;
			descriptorWrites[1].dstSet = myDescriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].pImageInfo = &imageInfo;
			descriptorWrites[1].pBufferInfo = nullptr;
			descriptorWrites[1].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(myLogicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void BasicRenderer::CreateCommandBuffers()
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

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.pNext = nullptr;
			renderPassInfo.renderPass = myRenderPass;
			renderPassInfo.framebuffer = mySwapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = mySwapChainExtent;
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(myCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myGraphicsPipeline);

			VkBuffer vertexBuffers[] = { myVertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(myCommandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(myCommandBuffers[i], myIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdBindDescriptorSets(myCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, myPipelineLayout, 0, 1, &myDescriptorSets[i], 0, nullptr);

			vkCmdDrawIndexed(myCommandBuffers[i], static_cast<uint32_t>(locIndices.size()), 1, 0, 0, 0);

			vkCmdEndRenderPass(myCommandBuffers[i]);

			if (vkEndCommandBuffer(myCommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to end a command buffer!");
			}
		}
	}

	void BasicRenderer::CreateSynchronizationObjects()
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

	void BasicRenderer::UpdateUniformBuffers(uint32_t anImageIndex)
	{
		// Using a UBO this way is not the most efficient way to pass frequently changing values to the shader.
		// A more efficient way to pass a small buffer of data to shaders are push constants.

		static auto startTime = std::chrono::high_resolution_clock::now();
		static float addTime = 0;

		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		Input::RawInputState spaceState = inputManager->PollRawInput(Input::RawInput::KeySpace);
		if (spaceState == Input::RawInputState::Pressed)
		{
			addTime += 0.1f;
		}

		auto currentTime = std::chrono::high_resolution_clock::now();
		float elapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count() + addTime;

		UniformBufferObject ubo{};
		ubo.myModel = glm::rotate(glm::mat4(1.0f), elapsedTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.myView = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.myProj = glm::perspective(glm::radians(45.0f), mySwapChainExtent.width / (float)mySwapChainExtent.height, 0.1f, 10.0f);
		ubo.myProj[1][1] *= -1; // adapt calculation for Vulkan

#if USE_VMA
		void* data;
		vmaMapMemory(myVmaAllocator, myUniformBuffersAllocs[anImageIndex], &data);
		memcpy(data, &ubo, sizeof(ubo));
		vmaUnmapMemory(myVmaAllocator, myUniformBuffersAllocs[anImageIndex]);
#else
		void* data;
		vkMapMemory(myLogicalDevice, myUniformBuffersMemory[anImageIndex], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(myLogicalDevice, myUniformBuffersMemory[anImageIndex]);
#endif
	}
}
