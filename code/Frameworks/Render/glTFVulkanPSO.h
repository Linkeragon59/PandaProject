#pragma once

namespace Render
{
namespace glTF
{
	class VulkanPSO
	{
	public:
		struct Vertex
		{
			glm::vec3 myPos;
			glm::vec3 myColor;
			glm::vec2 myTexCoord;
			glm::vec3 myNormal;
			glm::vec4 myJointIndices;
			glm::vec4 myJointWeights;

			static VkVertexInputBindingDescription GetBindingDescription()
			{
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;
				bindingDescription.stride = sizeof(Vertex);
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return bindingDescription;
			}

			static std::array<VkVertexInputAttributeDescription, 6> GetAttributeDescriptions()
			{
				std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};
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
				attributeDescriptions[3].location = 3;
				attributeDescriptions[3].binding = 0;
				attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[3].offset = offsetof(Vertex, myNormal);
				attributeDescriptions[4].location = 4;
				attributeDescriptions[4].binding = 0;
				attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
				attributeDescriptions[4].offset = offsetof(Vertex, myJointIndices);
				attributeDescriptions[5].location = 5;
				attributeDescriptions[5].binding = 0;
				attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
				attributeDescriptions[5].offset = offsetof(Vertex, myJointWeights);
				return attributeDescriptions;
			}
		};

		struct PerFrameUBO
		{
			glm::mat4 myView;
			glm::mat4 myProj;
			glm::vec4 myLightPos = glm::vec4(5.0f, 5.0f, 5.0f, 1.0f);
		};
		static VkDescriptorSetLayout ourPerFrameDescriptorSetLayout;

		struct PerObjectUBO
		{
			glm::mat4 myMatrix;
		};
		static VkDescriptorSetLayout ourPerObjectDescriptorSetLayout;

		static VkDescriptorSetLayout ourPerImageDescriptorSetLayout;

		static VkDescriptorSetLayout ourPerSkinDescriptorSetLayout;

		static void SetupDescriptorSetLayouts();
		static void DestroyDescriptorSetLayouts();

	public:
		VulkanPSO(VkRenderPass aRenderPass);
		~VulkanPSO();

		VkPipelineLayout GetPipelineLayout() const { return myPipelineLayout; }
		VkPipeline GetPipeline() const { return myPipeline; }

	private:
		void PreparePipeline();

		VkDevice myDevice = VK_NULL_HANDLE;
		VkRenderPass myRenderPass = VK_NULL_HANDLE;

		VkPipelineLayout myPipelineLayout = VK_NULL_HANDLE;
		VkPipeline myPipeline = VK_NULL_HANDLE;
	};
}
}
