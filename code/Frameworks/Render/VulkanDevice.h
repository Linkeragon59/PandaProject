#pragma once

#include "VulkanHelpers.h"
#include <optional>

namespace Render
{
	// Wrapper for Physical and Logical device
	struct VulkanDevice
	{
		explicit VulkanDevice(VkPhysicalDevice aPhysicalDevice);
		~VulkanDevice();

		void SetupLogicalDevice(
			const VkPhysicalDeviceFeatures& someEnabledFeatures,
			const std::vector<const char*>& someEnabledLayers,
			const std::vector<const char*>& someEnabledExtensions,
			VkQueueFlags someRequestedQueueTypes);

		void SetupVmaAllocator(VkInstance anInstance, uint32_t aVulkanAPIVersion);

		uint32_t GetQueueFamilyIndex(VkQueueFlagBits someQueueTypes) const;

		VkFormat FindSupportedFormat(const std::vector<VkFormat>& someCandidateFormats, VkImageTiling aTiling, VkFormatFeatureFlags someFeatures);
		VkFormat FindBestDepthFormat();

		VkPhysicalDevice myPhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties myProperties{};
		VkPhysicalDeviceFeatures myFeatures{};
		VkPhysicalDeviceMemoryProperties myMemoryProperties{};
		std::vector<VkQueueFamilyProperties> myQueueFamilyProperties;
		std::vector<VkExtensionProperties> mySupportedExtensions;

		VkDevice myLogicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceFeatures myEnabledFeatures{};
		struct QueueFamilyIndices
		{
			std::optional<uint32_t> myGraphicsFamily;
			std::optional<uint32_t> myComputeFamily;
			std::optional<uint32_t> myTransferFamily;
		} myQueueFamilyIndices;

		VkQueue myGraphicsQueue = VK_NULL_HANDLE;
		VkCommandPool myGraphicsCommandPool = VK_NULL_HANDLE;

		VmaAllocator myVmaAllocator = VK_NULL_HANDLE;

		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
		VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
		void            flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
		void            flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
	};
}
