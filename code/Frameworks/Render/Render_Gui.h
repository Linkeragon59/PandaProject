#pragma once

#include "Render_VulkanImage.h"
#include "Render_VulkanBuffer.h"
#include "Render_ShaderHelpers.h"

struct ImGuiContext;

namespace Render
{
	class Gui
	{
	public:
		Gui(ImGuiContext* aGuiContext);
		~Gui();

		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

	private:
		ImGuiContext* myGuiContext = nullptr;

		void PrepareFont();
		VulkanImage myFontTexture;

		ShaderHelpers::GuiPushConstBlock myPushConstBlock;

		VulkanBuffer myVertexBuffer;
		int myVertexCount = 0;
		VulkanBuffer myIndexBuffer;
		int myIndexCount = 0;
	};
}
