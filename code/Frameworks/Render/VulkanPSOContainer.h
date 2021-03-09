#pragma once

#include "VulkanHelpers.h"

#include "VulkanBuffer.h"

namespace Render
{
	class VulkanPSOContainer
	{
	public:
		VulkanPSOContainer(VkRenderPass aRenderPass);
		~VulkanPSOContainer();

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return myDescriptorSetLayout; }

		VkPipelineLayout GetPipelineLayout() const { return myPipelineLayout; }
		VkPipeline GetDefaultPipeline() const { return myDefaultPipeline; }

	private:
		void SetupDescriptorSetLayout();

		void CreatePipelineCache();
		void PreparePipelines();

		VkDevice myDevice = VK_NULL_HANDLE;
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		VkDescriptorSetLayout myDescriptorSetLayout = VK_NULL_HANDLE;

		VkPipelineCache myPipelineCache = VK_NULL_HANDLE;
		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
		VkPipeline myDefaultPipeline = VK_NULL_HANDLE;
	};
}
