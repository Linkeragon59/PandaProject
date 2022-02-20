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

		void Update();
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

		ImGuiContext* GetContext() const { return myGuiContext; }

	private:
		ImGuiContext* myGuiContext = nullptr;

		void PrepareFont();
		VulkanImagePtr myFontTexture;

		ShaderHelpers::GuiPushConstBlock myPushConstBlock;

		VulkanBufferPtr myVertexBuffer;
		int myVertexCount = 0;
		VulkanBufferPtr myIndexBuffer;
		int myIndexCount = 0;
	};
}
