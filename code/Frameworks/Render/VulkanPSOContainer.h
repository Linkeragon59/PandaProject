#pragma once

#include "VulkanHelpers.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Render
{
	class VulkanPSOContainer
	{
	public:
		VulkanPSOContainer(VkRenderPass aRenderPass);
		~VulkanPSOContainer();

		VkDescriptorPool GetDescriptorPool() const { return myDescriptorPool; }
		VkDescriptorSetLayout GetDescriptorSetLayout() const { return myDescriptorSetLayout; }

		VkPipelineLayout GetPipelineLayout() const { return myPipelineLayout; }

		VkPipeline GetPhongPipeline() const { return myPhongPipeline; }
		VkPipeline GetToonPipeline() const { return myToonPipeline; }
		VkPipeline GetWireFramePipeline() const { return myWireFramePipeline; }

		struct UBO
		{
			glm::mat4 myProjection;
			glm::mat4 myModelView;
			glm::vec4 myLightPos;
		};

	private:
		void SetupDescriptorPool();
		void SetupDescriptorSetLayout();

		void CreatePipelineCache();
		void PreparePipelines();

		VkDevice myDevice = VK_NULL_HANDLE;
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout myDescriptorSetLayout = VK_NULL_HANDLE;

		VkPipelineCache myPipelineCache = VK_NULL_HANDLE;

		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;

		VkPipeline myPhongPipeline = VK_NULL_HANDLE;
		VkPipeline myToonPipeline = VK_NULL_HANDLE;
		VkPipeline myWireFramePipeline = VK_NULL_HANDLE;
	};
}
