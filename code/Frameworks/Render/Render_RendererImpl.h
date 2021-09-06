#pragma once

#include "Render_Renderer.h"

namespace Render
{
	class SwapChain;
	class Camera;

	class RendererImpl
	{
	public:
		RendererImpl();
		virtual ~RendererImpl();

		virtual void Setup(SwapChain* aSwapChain);
		virtual void Cleanup();

		virtual void StartFrame();
		virtual void EndFrame();

		void SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection);
		virtual void AddLight(const PointLight& aPointLight) = 0;
		virtual void DrawModel(Handle aModelHandle, const ModelData& someData, Renderer::DrawType aDrawType) = 0;
		virtual void DrawGui(Handle aGuiHandle) = 0;

		VkSemaphore GetCurrentRenderFinishedSemaphore() const { return myRenderFinishedSemaphores[myCurrentFrameIndex]; }

	protected:
		VkDevice myDevice = VK_NULL_HANDLE;
		Camera* myCamera = nullptr;

		SwapChain* mySwapChain = nullptr;
		uint myCurrentFrameIndex = 0;

		// In-flight resources fences - one per frame
		void SetupSyncObjects();
		void DestroySyncObjects();
		std::vector<VkSemaphore> myRenderFinishedSemaphores;
		std::vector<VkFence> myFrameFences;

		// Command Buffers - one per frame
		virtual void SetupCommandBuffers();
		virtual void DestroyCommandBuffers();
		std::vector<VkCommandBuffer> myCommandBuffers;
	};
}
