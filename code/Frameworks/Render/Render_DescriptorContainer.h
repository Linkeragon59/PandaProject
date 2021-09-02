#pragma once

#include "Render_ShaderHelpers.h"

namespace Render
{
	struct DescriptorContainer
	{
		void Create(ShaderHelpers::BindType aType);
		void Destroy();

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return myLayout; }
		VkDescriptorSet GetDescriptorSet(const ShaderHelpers::DescriptorInfo& someDescriptorInfo, uint aFramesToKeep);

		void RecycleDescriptors();

		void AllocateDescriptorSet(VkDescriptorSet& anOutDescriptorSet);
		void UpdateDescriptorSet(const ShaderHelpers::DescriptorInfo& someDescriptorInfo, VkDescriptorSet aDescriptorSet);

		VkDevice myDevice = VK_NULL_HANDLE;

		ShaderHelpers::BindType myType = ShaderHelpers::BindType::Count;
		VkDescriptorSetLayout myLayout = VK_NULL_HANDLE;
		VkDescriptorPool myPool = VK_NULL_HANDLE;

		struct FrameAllocatedDescriptors
		{
			std::vector<VkDescriptorSet> mySets;
			uint myFirstAvailableSet = 0;
		};
		std::vector<FrameAllocatedDescriptors> myAllocatedSets;
		uint myCurrentFrame = 0;
	};
}
