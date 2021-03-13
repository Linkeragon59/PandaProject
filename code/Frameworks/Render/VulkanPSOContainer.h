#pragma once

namespace Render
{
	class VulkanPSOContainer
	{
	public:
		struct Vertex
		{
			glm::vec3 myPos;
			glm::vec3 myColor;
			glm::vec2 myTexCoord;

			static VkVertexInputBindingDescription GetBindingDescription()
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
				attributeDescriptions[0].location = 0;
				attributeDescriptions[0].binding = 0;
				attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[0].offset = offsetof(Vertex, myPos);
				attributeDescriptions[1].location = 1;
				attributeDescriptions[1].binding = 0;
				attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset = offsetof(Vertex, myColor);
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
				attributeDescriptions[2].offset = offsetof(Vertex, myTexCoord);
				return attributeDescriptions;
			}
		};

		struct PerFrameUBO
		{
			glm::mat4 myView;
			glm::mat4 myProj;
		};
		static VkDescriptorSetLayout ourPerFrameDescriptorSetLayout;

		struct PerObjectUBO
		{
			glm::mat4 myMatrix;
		};
		static VkDescriptorSetLayout ourPerObjectDescriptorSetLayout;

		static void SetupDescriptorSetLayouts();
		static void DestroyDescriptorSetLayouts();

	public:
		VulkanPSOContainer(VkRenderPass aRenderPass);
		~VulkanPSOContainer();

		VkPipelineLayout GetPipelineLayout() const { return myPipelineLayout; }
		VkPipeline GetDefaultPipeline() const { return myDefaultPipeline; }

	private:
		void CreatePipelineCache();
		void PreparePipelines();

		VkDevice myDevice = VK_NULL_HANDLE;
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		VkPipelineCache myPipelineCache = VK_NULL_HANDLE;

		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
		VkPipeline myDefaultPipeline = VK_NULL_HANDLE;
	};
}
