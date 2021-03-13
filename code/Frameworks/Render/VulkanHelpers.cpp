#include "VulkanHelpers.h"

#include "VulkanRenderer.h"

#include <fstream>

namespace Render
{
	namespace
	{
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

	bool CheckInstanceLayersSupport(const std::vector<const char*>& someLayers)
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (uint32_t i = 0, e = static_cast<uint32_t>(someLayers.size()); i < e; ++i)
		{
			bool supported = false;
			for (uint32_t j = 0; j < layerCount; ++j)
			{
				if (strcmp(someLayers[i], availableLayers[j].layerName) == 0)
				{
					supported = true;
					break;
				}
			}
			if (!supported)
				return false;
		}
		return true;
	}

	bool CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions)
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
				return false;
		}
		return true;
	}

	VkShaderModule CreateShaderModule(const std::string& aFilename)
	{
		VkShaderModule shaderModule;

		auto shaderCode = locReadFile(aFilename);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VK_CHECK_RESULT(vkCreateShaderModule(VulkanRenderer::GetInstance()->GetDevice(), &createInfo, nullptr, &shaderModule), "Failed to create a module shader!");

		return shaderModule;
	}

	VkCommandBuffer BeginOneTimeCommand(VkCommandPool aCommandPool)
	{
		if (aCommandPool == VK_NULL_HANDLE)
			aCommandPool = VulkanRenderer::GetInstance()->GetGraphicsCommandPool();

		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = aCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanRenderer::GetInstance()->GetDevice(), &allocInfo, &commandBuffer), "Failed to alloc a one time command buffer");

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin a one time command buffer");

		return commandBuffer;
	}

	void EndOneTimeCommand(VkCommandBuffer aCommandBuffer, VkQueue aQueue, VkCommandPool aCommandPool /*= VK_NULL_HANDLE*/)
	{
		if (aCommandPool == VK_NULL_HANDLE)
			aCommandPool = VulkanRenderer::GetInstance()->GetGraphicsCommandPool();

		VK_CHECK_RESULT(vkEndCommandBuffer(aCommandBuffer), "Failed to end a one time command buffer");

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &aCommandBuffer;

		vkQueueSubmit(aQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(aQueue);

		vkFreeCommandBuffers(VulkanRenderer::GetInstance()->GetDevice(), aCommandPool, 1, &aCommandBuffer);
	}
}