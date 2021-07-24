#pragma once

#include <optional>

namespace Render::Vulkan
{
	// Wrapper for Physical and Logical device
	struct Device
	{
		explicit Device(VkPhysicalDevice aPhysicalDevice);
		~Device();

		bool SupportsExtension(const char* anExtension);

		void SetupLogicalDevice(
			const VkPhysicalDeviceFeatures& someEnabledFeatures,
			const std::vector<const char*>& someEnabledLayers,
			const std::vector<const char*>& someEnabledExtensions,
			VkQueueFlags someRequestedQueueTypes);

		void SetupVmaAllocator(VkInstance anInstance, uint aVulkanAPIVersion);

		uint GetQueueFamilyIndex(VkQueueFlagBits someQueueTypes) const;

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
			std::optional<uint> myGraphicsFamily;
			std::optional<uint> myComputeFamily;
			std::optional<uint> myTransferFamily;
		} myQueueFamilyIndices;

		VkQueue myGraphicsQueue = VK_NULL_HANDLE;
		VkCommandPool myGraphicsCommandPool = VK_NULL_HANDLE;

		VmaAllocator myVmaAllocator = VK_NULL_HANDLE;
	};
}
