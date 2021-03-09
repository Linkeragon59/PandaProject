#pragma once

#include "VulkanHelpers.h"
#include "VulkanBuffer.h"

namespace Render
{
	class VulkanCamera
	{
	public:
		VulkanCamera(VkDescriptorSetLayout aDescriptorSetLayout);
		~VulkanCamera();

		VkDescriptorSet GetDescriptorSet() const { return myDescriptorSet; }

		struct UniformBufferObject
		{
			glm::mat4 myModel;
			glm::mat4 myView;
			glm::mat4 myProj;
		};

	private:
		void SetupDescriptorPool();

		void PrepareUniformBuffers();
		void SetupDescriptorSet();

		VkDevice myDevice = VK_NULL_HANDLE;
		VkDescriptorSetLayout myDescriptorSetLayout = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		// TODO: Maybe should have one per image, not sure if that's useful
		VulkanBuffer myUBO;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
