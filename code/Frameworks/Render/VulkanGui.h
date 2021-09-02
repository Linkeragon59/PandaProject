#pragma once

#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanShaderHelpers.h"

namespace Render::Vulkan
{
	class Gui
	{
	public:
		Gui();
		~Gui();

		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

	private:
		void PrepareFont();
		Image myFontTexture;

		ShaderHelpers::GuiPushConstBlock myPushConstBlock;

		Buffer myVertexBuffer;
		int myVertexCount = 0;
		Buffer myIndexBuffer;
		int myIndexCount = 0;
	};
}
