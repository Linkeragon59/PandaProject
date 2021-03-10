#pragma once

#include "VulkanHelpers.h"
#include "VulkanBuffer.h"

namespace Render
{
	class VulkanCamera
	{
	public:
		VulkanCamera();
		~VulkanCamera();

		VkDescriptorSet GetDescriptorSet() const { return myDescriptorSet; }

		static void SetupDescriptorSetLayout();
		static void DestroyDescriptorSetLayout();
		static VkDescriptorSetLayout GetDescriptorSetLayout() { return ourDescriptorSetLayout; }

	private:
		static VkDescriptorSetLayout ourDescriptorSetLayout;
		
		void SetupDescriptorPool();

		void PrepareUniformBuffers();
		void SetupDescriptorSet();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		// TODO: Maybe should have one per image, not sure if that's useful
		struct UBO
		{
			glm::mat4 myView;
			glm::mat4 myProj;
		};
		VulkanBuffer myUBO;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
