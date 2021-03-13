#pragma once

#include "VulkanBuffer.h"

namespace Render
{
	class VulkanCamera
	{
	public:
		VulkanCamera();
		~VulkanCamera();

		void Update();

		VkDescriptorSet GetDescriptorSet() const { return myDescriptorSet; }

	private:		
		void SetupDescriptorPool();

		void PrepareUniformBuffers();
		void SetupDescriptorSet();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		glm::vec3 myPosition;
		glm::vec3 myDirection;
		VulkanBuffer myUBO;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
