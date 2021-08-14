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
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::DescriptorLayout aLayout) override;

	private:
		Buffer myVertexBuffer;
		Buffer myIndexBuffer;
		uint myIndexCount = 0;

		Buffer myUBOObject;
		Image myTexture;

		void SetupSimpleDescriptorSet();
		void SetupDescriptorSet();
		VkDescriptorSet mySimpleDescriptorSet = VK_NULL_HANDLE;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
