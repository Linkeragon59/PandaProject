#pragma once

#include "Render_VulkanImage.h"
#include "Render_VulkanBuffer.h"
#include "Render_ShaderHelpers.h"

namespace Render
{
	class Gui
	{
	public:
		Gui();
		~Gui();

		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

	private:
		void PrepareFont();
		VulkanImage myFontTexture;

		ShaderHelpers::GuiPushConstBlock myPushConstBlock;

		VulkanBuffer myVertexBuffer;
		int myVertexCount = 0;
		VulkanBuffer myIndexBuffer;
		int myIndexCount = 0;
	};
}
