#pragma once

#include "VulkanModel.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render::Vulkan
{
	/*class DummyModel : public Model
	{
	public:
		DummyModel();
		~DummyModel();

		void Update() override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) override;

	private:
		void SetupDescriptorPool();

		void PrepareBuffers();
		void SetupDescriptoSets();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		Buffer myVertexBuffer;
		Buffer myIndexBuffer;
		uint myIndexCount = 0;

		Buffer myUBOObject;
		Buffer mySSBOSkin;
		Image myTexture;
		Buffer mySSBOMaterial;

		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};*/
}
