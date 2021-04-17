#pragma once

#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render
{
namespace Vulkan
{
	class DummyModel
	{
	public:
		DummyModel(const glm::vec3& aPosition);
		~DummyModel();

		void Update();

		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout);

	private:
		void SetupDescriptorPool();

		void PrepareBuffers();
		void SetupDescriptoSets();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		Buffer myVertexBuffer;
		Buffer myIndexBuffer;
		uint32_t myIndexCount = 0;

		glm::vec3 myPosition;
		Buffer myUBOObject;
		VkDescriptorSet myDescriptorSetObject = VK_NULL_HANDLE;

		Image myTexture;
		VkDescriptorSet myDescriptorSetImage = VK_NULL_HANDLE;

		Buffer mySSBOMaterial;
		VkDescriptorSet myDescriptorSetMaterial = VK_NULL_HANDLE;

		Buffer mySSBOSkin;
		VkDescriptorSet myDescriptorSetSkin = VK_NULL_HANDLE;
	};
}
}
