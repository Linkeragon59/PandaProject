#pragma once

#include "vulkan/vulkan.h"

#define USE_VMA 0

#if USE_VMA
#include "vk_mem_alloc.h"
#endif

#include <vector>
#include <optional>

struct GLFWwindow;

namespace Render
{
	class BasicRendererTuto
	{
	public:
		BasicRendererTuto();
		~BasicRendererTuto();

		void Run();

	private:
		void InitWindow();
		void InitVulkan();

		void DrawFrame();

		void Cleanup();

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR myCapabilities = {};
			std::vector<VkSurfaceFormatKHR> myFormats;
			std::vector<VkPresentModeKHR> myPresentModes;
		};

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> myGraphicsFamily;
			std::optional<uint32_t> myPresentFamily;

			bool IsComplete() const
			{
				return myGraphicsFamily.has_value()
					&& myPresentFamily.has_value();
			}
		};

		static bool CheckValidationLayersSupport(const std::vector<const char*>& someValidationLayers);
		static bool CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions);
		static bool CheckDeviceExtensionsSupport(VkPhysicalDevice aPhysicalDevice, const std::vector<const char*>& someExtensions);
#if !USE_VMA
		static uint32_t FindMemoryType(VkPhysicalDevice aPhysicalDevice, uint32_t aTypeFilter, VkMemoryPropertyFlags someProperties);
#endif
		static bool IsDeviceSuitable(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface);
		static void QuerySwapChainSupport(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface, SwapChainSupportDetails& someOutDetails);
		static void FindQueueFamilies(VkPhysicalDevice aPhysicalDevice, VkSurfaceKHR aSurface, QueueFamilyIndices& someOutQueueIndices);
		static VkFormat FindSupportedFormat(VkPhysicalDevice aPhysicalDevice, const std::vector<VkFormat>& someCandidateFormats, VkImageTiling aTiling, VkFormatFeatureFlags someFeatures);
		static VkFormat FindBestDepthFormat(VkPhysicalDevice aPhysicalDevice);

		static VkExtent2D SelectSwapChainExtent(const VkSurfaceCapabilitiesKHR& someCapabilities);
		static VkSurfaceFormatKHR SelectSwapChainFormat(const std::vector<VkSurfaceFormatKHR>& someAvailableFormats);
		static VkPresentModeKHR SelectSwapChainPresentMode(const std::vector<VkPresentModeKHR>& someAvailablePresentModes);
		
		static VkShaderModule CreateShaderModule(VkDevice aLogicalDevice, const std::vector<char>& someByteCode);

		static VkCommandBuffer BeginOneShotCommand(VkDevice aLogicalDevice, VkCommandPool aCommandPool);
		static void EndOneShotCommand(VkDevice aLogicalDevice, VkCommandPool aCommandPool, VkQueue aQueue, VkCommandBuffer aCommandBuffer);
		static void TransitionImageLayout(VkDevice aLogicalDevice, VkCommandPool aCommandPool, VkQueue aQueue, VkImage anImage, VkFormat aFormat, VkImageLayout anOldLayout, VkImageLayout aNewLayout);
		
#if USE_VMA
		static void CreateImage(VmaAllocator aVmaAllocator, uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkImage& anOutImage, VmaAllocation& anOutImageAlloc);
		static void CreateBuffer(VmaAllocator aVmaAllocator, VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkBuffer& anOutBuffer, VmaAllocation& anOutBufferAlloc);
#else
		static void CreateImage(VkPhysicalDevice aPhysicalDevice, VkDevice aLogicalDevice, uint32_t aWidth, uint32_t aHeight, VkFormat aFormat, VkImageTiling aTiling, VkImageUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkImage& anOutImage, VkDeviceMemory& anOutImageMemory);
		static void CreateBuffer(VkPhysicalDevice aPhysicalDevice, VkDevice aLogicalDevice, VkDeviceSize aSize, VkBufferUsageFlags aUsage, VkMemoryPropertyFlags someProperties, VkBuffer& anOutBuffer, VkDeviceMemory& anOutBufferMemory);
#endif

		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& aCreateInfo);

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT aMessageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT aMessageType,
			const VkDebugUtilsMessengerCallbackDataEXT* aCallbackData,
			void* aUserData);

		static void FramebufferResizeCallback(
			GLFWwindow* aWindow,
			int aWidth,
			int aHeight);

		void CreateInstance();
		void SetupDebugMessenger();
		void CreateSurface();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
#if USE_VMA
		void CreateVmaAllocator();
#endif
		void CreateSwapChain();
		void RecreateSwapChain();
		void CleanupSwapChain();
		void CreateSwapChainImageViews();
		void CreateRenderPass();
		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline();
		void CreateFrameBuffers();
		void CreateCommandPool();
		void CreateDepthResources();
		void CreateTextureImage();
		void CreateTextureImageView();
		void CreateTextureSampler();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateCommandBuffers();
		void CreateSynchronizationObjects();

		void UpdateUniformBuffers(uint32_t anImageIndex);

		GLFWwindow* myWindow = nullptr;

		VkInstance myInstance = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT myDebugCallback = VK_NULL_HANDLE;

		VkSurfaceKHR mySurface = VK_NULL_HANDLE;
		VkPhysicalDevice myPhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceFeatures myAvailableFeatures{};
		VkPhysicalDeviceFeatures myRequestedFeatures{};
		VkDevice myLogicalDevice = VK_NULL_HANDLE;

#if USE_VMA
		VmaAllocator myVmaAllocator = VK_NULL_HANDLE;
#endif

		VkQueue myGraphicsQueue = VK_NULL_HANDLE;
		VkQueue myPresentQueue = VK_NULL_HANDLE;

		VkSwapchainKHR mySwapChain = VK_NULL_HANDLE;
		std::vector<VkImage> mySwapChainImages;
		std::vector<VkImageView> mySwapChainImageViews;
		std::vector<VkFramebuffer> mySwapChainFramebuffers;
		VkFormat mySwapChainFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D mySwapChainExtent = VkExtent2D{ 0, 0 };

		VkRenderPass myRenderPass = VK_NULL_HANDLE;
		VkDescriptorSetLayout myDescriptorSetLayout;
		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
		VkPipeline myGraphicsPipeline = VK_NULL_HANDLE;

		VkCommandPool myCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> myCommandBuffers;

		VkImage myDepthImage = VK_NULL_HANDLE;
#if USE_VMA
		VmaAllocation myDepthImageAlloc = VK_NULL_HANDLE;
#else
		VkDeviceMemory myDepthImageMemory = VK_NULL_HANDLE;
#endif
		VkImageView myDepthImageView = VK_NULL_HANDLE;

		VkImage myTextureImage = VK_NULL_HANDLE;
#if USE_VMA
		VmaAllocation myTextureImageAlloc = VK_NULL_HANDLE;
#else
		VkDeviceMemory myTextureImageMemory = VK_NULL_HANDLE;
#endif
		VkImageView myTextureImageView = VK_NULL_HANDLE;

		VkSampler myTextureSampler = VK_NULL_HANDLE;

		VkBuffer myVertexBuffer = VK_NULL_HANDLE;
#if USE_VMA
		VmaAllocation myVertexBufferAlloc = VK_NULL_HANDLE;
#else
		VkDeviceMemory myVertexBufferMemory = VK_NULL_HANDLE;
#endif
		VkBuffer myIndexBuffer = VK_NULL_HANDLE;
#if USE_VMA
		VmaAllocation myIndexBufferAlloc = VK_NULL_HANDLE;
#else
		VkDeviceMemory myIndexBufferMemory = VK_NULL_HANDLE;
#endif

		std::vector<VkBuffer> myUniformBuffers;
#if USE_VMA
		std::vector<VmaAllocation> myUniformBuffersAllocs;
#else
		std::vector<VkDeviceMemory> myUniformBuffersMemory;
#endif

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> myDescriptorSets;

		std::vector<VkSemaphore> myImageAvailableSemaphores;
		std::vector<VkSemaphore> myRenderFinishedSemaphores;
		std::vector<VkFence> myInFlightFrameFences;
		std::vector<VkFence> myImageFences;
		uint32_t myCurrentInFlightFrame = 0;

		bool myFramebufferResized = false;
	};
}
