#pragma once

#include "VulkanModel.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render::Vulkan
{
	class SimpleGeometryModel : public Model
	{
	public:
		SimpleGeometryModel(const ModelData& someData);
		~SimpleGeometryModel();

		void Update(const ModelData& someData) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) override;

	private:
		Buffer myVertexBuffer;
		Buffer myIndexBuffer;
		uint myIndexCount = 0;

		Buffer myUBOObject;
		Image myTexture;
	};
}
