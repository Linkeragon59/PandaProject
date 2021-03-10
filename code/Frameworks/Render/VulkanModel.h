#pragma once

#include "VulkanHelpers.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"

namespace Render
{
	class VulkanModel
	{
	public:
		VulkanModel();
		~VulkanModel();

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

		static void SetupDescriptorSetLayout();
		static void DestroyDescriptorSetLayout();
		static VkDescriptorSetLayout GetDescriptorSetLayout() { return ourDescriptorSetLayout; }

		VkDescriptorSet GetDescriptorSet() const { return myDescriptorSet; }
		VkBuffer GetVertexBuffer() const { return myVertexBuffer.myBuffer; }
		VkBuffer GetIndexBuffer() const { return myIndexBuffer.myBuffer; }
		uint32_t GetIndexCount() const { return myIndexCount; }

	private:
		static VkDescriptorSetLayout ourDescriptorSetLayout;

		void SetupDescriptorPool();

		void PrepareUniformBuffers();
		void SetupDescriptoSet();

		VkDevice myDevice = VK_NULL_HANDLE;

		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		VulkanBuffer myVertexBuffer;
		VulkanBuffer myIndexBuffer;
		uint32_t myIndexCount = 0;

		VulkanImage myTexture;

		struct UBO
		{
			glm::mat4 myModel;
		};
		VulkanBuffer myUBO;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
