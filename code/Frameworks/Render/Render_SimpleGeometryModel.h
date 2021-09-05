#pragma once

#include "Render_ModelImpl.h"
#include "Render_VulkanBuffer.h"
#include "Render_VulkanImage.h"

namespace Render
{
	class SimpleGeometryModel : public ModelImpl
	{
	public:
		SimpleGeometryModel(const ModelData& someData);

		void Update(const ModelData& someData) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) override;

	private:
		VulkanBufferPtr myVertexBuffer;
		VulkanBufferPtr myIndexBuffer;
		uint myIndexCount = 0;

		VulkanBufferPtr myUBOObject;
		VulkanImagePtr myTexture;
	};
}
