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
		~SimpleGeometryModel();

		void Update(const ModelData& someData) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) override;

	private:
		VulkanBuffer myVertexBuffer;
		VulkanBuffer myIndexBuffer;
		uint myIndexCount = 0;

		VulkanBuffer myUBOObject;
		VulkanImage myTexture;
	};
}
