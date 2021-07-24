#pragma once

#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render::Vulkan
{
	class ImGuiOverlay
	{
	public:
		ImGuiOverlay();
		~ImGuiOverlay();

		void Prepare(VkRenderPass aRenderPass);
		void Destroy();

		void Draw(const VkCommandBuffer aCommandBuffer);

	private:
		VkDevice myDevice = VK_NULL_HANDLE;

		Image myFontTexture;

		struct PushConstBlock {
			glm::vec2 myScale;
			glm::vec2 myTranslate;
		} myPushConstBlock;

		Buffer myVertexBuffer;
		int myVertexCount = 0;
		Buffer myIndexBuffer;
		int myIndexCount = 0;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;
		VkDescriptorSetLayout myDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;

		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
		VkPipeline myPipeline = VK_NULL_HANDLE;

		void PrepareFont();
		void SetupDescriptorPool();
		void SetupDescriptorSetLayout();
		void SetupDescriptorSet();
		void SetupPipeline(VkRenderPass aRenderPass);
	};
}
