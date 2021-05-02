#pragma once

#define VK_CHECK_RESULT(X, Msg) Verify((X == VK_SUCCESS), Msg)

namespace Render
{
namespace Vulkan
{
	bool CheckInstanceLayersSupport(const std::vector<const char*>& someLayers);
	bool CheckInstanceExtensionsSupport(const std::vector<const char*>& someExtensions);

	VkShaderModule CreateShaderModule(const std::string& aFilename);

	VkCommandBuffer BeginOneTimeCommand(VkCommandPool aCommandPool = VK_NULL_HANDLE);
	void EndOneTimeCommand(VkCommandBuffer aCommandBuffer, VkQueue aQueue, VkCommandPool aCommandPool = VK_NULL_HANDLE);
}
}
