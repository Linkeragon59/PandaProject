#include "VulkanDevice.h"

#include "VulkanHelpers.h"

namespace Render
{
namespace Vulkan
{
	Device::Device(VkPhysicalDevice aPhysicalDevice)
		: myPhysicalDevice(aPhysicalDevice)
	{
		assert(myPhysicalDevice);

		vkGetPhysicalDeviceProperties(myPhysicalDevice, &myProperties);
		vkGetPhysicalDeviceFeatures(myPhysicalDevice, &myFeatures);
		vkGetPhysicalDeviceMemoryProperties(myPhysicalDevice, &myMemoryProperties);

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		myQueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(myPhysicalDevice, &queueFamilyCount, myQueueFamilyProperties.data());

		uint32_t extensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(myPhysicalDevice, nullptr, &extensionsCount, nullptr);
		if (extensionsCount > 0)
		{
			mySupportedExtensions.resize(extensionsCount);
			vkEnumerateDeviceExtensionProperties(myPhysicalDevice, nullptr, &extensionsCount, mySupportedExtensions.data());
		}
	}

	Device::~Device()
	{
		if (myLogicalDevice)
		{
			vkDeviceWaitIdle(myLogicalDevice);

			if (myVmaAllocator)
				vmaDestroyAllocator(myVmaAllocator);

			vkDestroyCommandPool(myLogicalDevice, myGraphicsCommandPool, nullptr);
			vkDestroyDevice(myLogicalDevice, nullptr);
		}
	}

	void Device::SetupLogicalDevice(
		const VkPhysicalDeviceFeatures& someEnabledFeatures,
		const std::vector<const char*>& someEnabledLayers,
		const std::vector<const char*>& someEnabledExtensions,
		VkQueueFlags someRequestedQueueTypes)
	{
		myEnabledFeatures = someEnabledFeatures;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (someRequestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			myQueueFamilyIndices.myGraphicsFamily = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);

			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = myQueueFamilyIndices.myGraphicsFamily.value();
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}

		// Compute queue
		if (someRequestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			myQueueFamilyIndices.myComputeFamily = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);

			if (myQueueFamilyIndices.myComputeFamily != myQueueFamilyIndices.myGraphicsFamily)
			{
				// We need an additional queue for dedicated compute
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = myQueueFamilyIndices.myComputeFamily.value();
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}

		// Transfer queue
		if (someRequestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			myQueueFamilyIndices.myTransferFamily = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);

			if ((myQueueFamilyIndices.myTransferFamily != myQueueFamilyIndices.myGraphicsFamily)
			&& (myQueueFamilyIndices.myTransferFamily != myQueueFamilyIndices.myComputeFamily))
			{
				// We need an additional queue for dedicated transfer
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = myQueueFamilyIndices.myTransferFamily.value();
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}

		// Logical Device creation
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(someEnabledLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = someEnabledLayers.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(someEnabledExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = someEnabledExtensions.data();
		deviceCreateInfo.pEnabledFeatures = &myEnabledFeatures;

		VK_CHECK_RESULT(vkCreateDevice(myPhysicalDevice, &deviceCreateInfo, nullptr, &myLogicalDevice), "Could not create the logical device");

		if (myQueueFamilyIndices.myGraphicsFamily.has_value())
		{
			vkGetDeviceQueue(myLogicalDevice, myQueueFamilyIndices.myGraphicsFamily.value(), 0, &myGraphicsQueue);

			VkCommandPoolCreateInfo commandPoolInfo{};
			commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolInfo.queueFamilyIndex = myQueueFamilyIndices.myGraphicsFamily.value();

			VK_CHECK_RESULT(vkCreateCommandPool(myLogicalDevice, &commandPoolInfo, nullptr, &myGraphicsCommandPool), "Failed to create the graphics command pool");
		}
	}

	void Device::SetupVmaAllocator(VkInstance anInstance, uint32_t aVulkanApiVersion)
	{
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.instance = anInstance;
		allocatorInfo.physicalDevice = myPhysicalDevice;
		allocatorInfo.device = myLogicalDevice;
		allocatorInfo.vulkanApiVersion = aVulkanApiVersion;

		VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &myVmaAllocator), "Could not create the vma allocator");
	}

	uint32_t Device::GetQueueFamilyIndex(VkQueueFlagBits someQueueTypes) const
	{
		if ((someQueueTypes & VK_QUEUE_COMPUTE_BIT) && !(someQueueTypes & VK_QUEUE_GRAPHICS_BIT))
		{
			// Dedicated queue for compute
			// Try to find a queue family index that supports compute but not graphics
			for (uint32_t i = 0; i < static_cast<uint32_t>(myQueueFamilyProperties.size()); ++i)
			{
				VkQueueFlags flags = myQueueFamilyProperties[i].queueFlags;
				if ((flags & someQueueTypes) && !(flags & VK_QUEUE_GRAPHICS_BIT))
					return i;
			}
		}

		if ((someQueueTypes & VK_QUEUE_TRANSFER_BIT) && !(someQueueTypes & VK_QUEUE_GRAPHICS_BIT) && !(someQueueTypes & VK_QUEUE_COMPUTE_BIT))
		{
			// Dedicated queue for transfer
			// Try to find a queue family index that supports transfer but not graphics and compute
			for (uint32_t i = 0; i < static_cast<uint32_t>(myQueueFamilyProperties.size()); ++i)
			{
				VkQueueFlags flags = myQueueFamilyProperties[i].queueFlags;
				if ((flags & someQueueTypes) && !(flags & VK_QUEUE_GRAPHICS_BIT) && !(flags & VK_QUEUE_COMPUTE_BIT))
					return i;
			}
		}

		// For other queue types, return the first one to support the requested flags
		for (uint32_t i = 0; i < static_cast<uint32_t>(myQueueFamilyProperties.size()); ++i)
		{
			if (myQueueFamilyProperties[i].queueFlags & someQueueTypes)
				return i;
		}

		throw std::runtime_error("Couldn't find a matching queue family index");
	}

	VkFormat Device::FindSupportedFormat(const std::vector<VkFormat>& someCandidateFormats, VkImageTiling aTiling, VkFormatFeatureFlags someFeatures)
	{
		for (VkFormat format : someCandidateFormats)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(myPhysicalDevice, format, &props);

			if (aTiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & someFeatures) == someFeatures)
				return format;
			else if (aTiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & someFeatures) == someFeatures)
				return format;
		}

		throw std::runtime_error("Couldn't find an available format!");
	}

	VkFormat Device::FindBestDepthFormat()
	{
		// Start with the highest precision packed format
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		return FindSupportedFormat(depthFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}
}
