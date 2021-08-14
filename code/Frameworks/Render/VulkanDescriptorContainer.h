#pragma once

#include "VulkanShaderHelpers.h"

namespace Render::Vulkan
{
	struct DescriptorContainer
	{
		void Create(ShaderHelpers::DescriptorLayout aLayout);
		void Destroy();

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return myLayout; }
		void AllocateDescriptorSet(VkDescriptorSet& anOutDescriptorSet);
		void UpdateDescriptorSet(const ShaderHelpers::DescriptorInfo& someDescriptorInfo, VkDescriptorSet aDescriptorSet);

		VkDevice myDevice = VK_NULL_HANDLE;

		ShaderHelpers::DescriptorLayout myLayoutType = ShaderHelpers::DescriptorLayout::Count;
		VkDescriptorSetLayout myLayout = VK_NULL_HANDLE;
		VkDescriptorPool myPool = VK_NULL_HANDLE;
	};
}
