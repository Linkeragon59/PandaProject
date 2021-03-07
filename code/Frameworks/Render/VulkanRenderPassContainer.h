#pragma once

#include "VulkanHelpers.h"

namespace Render
{
	class VulkanRenderPassContainer
	{
	public:
		VulkanRenderPassContainer(VkFormat aSwapChainColorFormat, VkFormat aSwapChainDepthFormat);
		~VulkanRenderPassContainer();

		VkRenderPass GetRenderPass() const { return myRenderPass; }

	private:
		void SetupRenderPass();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkFormat mySwapChainColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat mySwapChainDepthFormat = VK_FORMAT_UNDEFINED;
		VkRenderPass myRenderPass = VK_NULL_HANDLE;
	};
}
