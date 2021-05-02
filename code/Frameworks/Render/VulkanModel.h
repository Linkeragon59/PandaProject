#pragma once

namespace Render
{
namespace Vulkan
{
	class Model
	{
	public:
		virtual ~Model() {};

		virtual void Update() = 0;
		virtual void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout) = 0;
	};
}
}
