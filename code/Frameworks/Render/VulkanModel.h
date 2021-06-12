#pragma once

#include "RenderData.h"

namespace Render
{
namespace Vulkan
{
	class Model
	{
	public:
		Model(const RenderData& aRenderData)
		{
			myRenderData = aRenderData;
		}

		virtual ~Model() {};

		virtual void Update() = 0;
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex) = 0;

		glm::vec3 GetPosition() const { return myRenderData.myMatrix[3]; }

		bool IsTransparent() const { return myRenderData.myIsTransparent; }

	protected:
		RenderData myRenderData;
	};
}
}
