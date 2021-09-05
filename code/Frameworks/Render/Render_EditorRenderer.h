#pragma once

#if DEBUG_BUILD

#include "Render_RendererImpl.h"
#include "Render_GuiPipeline.h"

namespace Render
{
	class EditorRenderer : public RendererImpl
	{
	public:
		void Setup(SwapChain* aSwapChain) override;
		void Cleanup() override;

		void StartFrame() override;
		void EndFrame() override;

		void SetViewport(const VkViewport& aViewport);
		void SetScissor(const VkRect2D& aScissor);

		void DrawModel(Model* /*aModel*/, const ModelData& /*someData*/, DrawType /*aDrawType*/ = DrawType::Default) override {};
		void AddLight(const PointLight& /*aPointLight*/) override {};
		void DrawGui(Gui* aGui) override;

	private:
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;

		// Render Pass
		void SetupRenderPass();
		void DestroyRenderPass();
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		// Pipelines
		void SetupPipeline();
		void DestroyPipeline();
		GuiPipeline myGuiPipeline;

		// Command Buffers - one per frame
		void SetupCommandBuffers() override;
		void DestroyCommandBuffers() override;
		std::vector<VkCommandBuffer> mySecondaryCommandBuffersGui;

		// Frame Buffers - one per frame
		void SetupFrameBuffers();
		void DestroyFrameBuffers();
		std::vector<VkFramebuffer> myFrameBuffers;
	};
}

#endif
