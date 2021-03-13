#pragma once

#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render
{
	class VulkanModel
	{
	public:
		VulkanModel(const glm::vec3& aPosition);
		~VulkanModel();

		void Update();

		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout);

	private:
		void SetupDescriptorPool();

		void PrepareUniformBuffers();
		void SetupDescriptoSet();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		VulkanBuffer myVertexBuffer;
		VulkanBuffer myIndexBuffer;
		uint32_t myIndexCount = 0;

		VulkanImage myTexture;

		glm::vec3 myPosition;
		VulkanBuffer myUBO;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
