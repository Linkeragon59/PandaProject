#pragma once

#include "VulkanImage.h"

namespace Render
{
namespace Vulkan
{
	struct DeferredRenderPass
	{
		void Setup(VkExtent2D anExtent, VkFormat aColorFormat, VkFormat aDepthFormat);
		void Destroy();

		Image myPositionAttachement;
		Image myNormalAttachement;
		Image myAlbedoAttachement;

		VkRenderPass myRenderPass = VK_NULL_HANDLE;
	};
}
}
