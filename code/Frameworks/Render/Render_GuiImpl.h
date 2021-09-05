#pragma once

#include "Render_Gui.h"
#include "Render_VulkanImage.h"
#include "Render_VulkanBuffer.h"
#include "Render_ShaderHelpers.h"

struct ImGuiContext;

namespace Render
{
	class GuiImpl : public Gui
	{
	public:
		GuiImpl(ImGuiContext* aGuiContext);

		void Update() override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

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
