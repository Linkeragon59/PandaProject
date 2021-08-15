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
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) override;

	private:
		Buffer myVertexBuffer;
		Buffer myIndexBuffer;
		uint myIndexCount = 0;

		Buffer myUBOObject;
		Image myTexture;
	};
}
