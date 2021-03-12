#pragma once

#define VK_CHECK_RESULT(X, Msg) if (X != VK_SUCCESS) { throw std::runtime_error(Msg); }

namespace Render
{
	bool CheckInstanceLayersSupport(const std::vector<const char*>& someLayers);
	bool CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions);

	VkShaderModule CreateShaderModule(const std::string& aFilename);

	VkCommandBuffer BeginOneTimeCommand(VkCommandPool aCommandPool = VK_NULL_HANDLE);
	void EndOneTimeCommand(VkCommandBuffer aCommandBuffer, VkQueue aQueue, VkCommandPool aCommandPool = VK_NULL_HANDLE);
}
