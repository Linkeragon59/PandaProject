#pragma once

#include "VulkanModel.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render::Vulkan
{
	class DynamicModel : public Model
	{
	public:
		DynamicModel(const BaseModelData& someData);
		~DynamicModel();

		void Update(const BaseModelData& someData) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) const override;

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
	};
}
