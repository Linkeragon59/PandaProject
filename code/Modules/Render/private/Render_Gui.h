#pragma once

#include "Render_VulkanImage.h"
#include "Render_VulkanBuffer.h"
#include "Render_ShaderHelpers.h"

#include <queue>

struct ImGuiContext;

namespace Render
{
	class Gui
	{
	public:
		Gui();
		~Gui();

		void Update();
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

		ImGuiContext* GetContext() const { return myGuiContext; }

	private:
		ImGuiContext* myGuiContext = nullptr;
		GLFWwindow* myWindow = nullptr;

		uint myWindowResizeCallbackId = UINT_MAX;
		int myWindowWidth = -1;
		int myWindowHeight = -1;

		uint myScrollCallbackId = UINT_MAX;
		double myXScroll = 0.0;
		double myYScroll = 0.0;

		uint myCharacterCallbackId = UINT_MAX;
		std::queue<uint> myTextInput;

		void PrepareFont();
		VulkanImagePtr myFontTexture;

		ShaderHelpers::GuiPushConstBlock myPushConstBlock;

		VulkanBufferPtr myVertexBuffer;
		int myVertexCount = 0;
		VulkanBufferPtr myIndexBuffer;
		int myIndexCount = 0;

		void InitStyle();
		void InitIO();
	};
}
