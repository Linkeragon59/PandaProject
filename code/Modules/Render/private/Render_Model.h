#pragma once

#include "Render_ShaderHelpers.h"
#include "Render_EntityRenderComponent.h"

namespace Render
{
	class Model
	{
	public:
		virtual ~Model() {};
		virtual void Update(const glm::mat4& aMatrix) = 0;
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) = 0;
	};

	class SimpleGeometryModel : public Model
	{
	public:
		SimpleGeometryModel(const std::vector<EntitySimpleGeometryModelComponent::Vertex>& someVertices, const std::vector<uint>& someIndices, const std::string& aTextureFilename);

		void Update(const glm::mat4& aMatrix) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) override;

	private:
		VulkanBufferPtr myVertexBuffer;
		VulkanBufferPtr myIndexBuffer;
		uint myIndexCount = 0;

		VulkanBufferPtr myUBOObject;
		VulkanImagePtr myTexture;
	};
}