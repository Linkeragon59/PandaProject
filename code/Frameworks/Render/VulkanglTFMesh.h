#pragma once

#include "VulkanHelpers.h"
#include "VulkanglTFMaterial.h"
#include "VulkanBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <string>

namespace Render
{
namespace VulkanglTF
{
	enum class VertexComponent
	{
		Position,
		Normal,
		UV,
		Color
	};

	struct Vertex
	{
		glm::vec3 myPosition;
		glm::vec3 myNormal;
		glm::vec2 myUV;
		glm::vec4 myColor;

		static VkVertexInputBindingDescription GetBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static VkVertexInputAttributeDescription GetAttributeDescription(VertexComponent aComponent, uint32_t aLocation)
		{
			VkVertexInputAttributeDescription description;
			description.location = aLocation;
			description.binding = 0;
			switch (aComponent)
			{
			case VertexComponent::Position:
				description.format = VK_FORMAT_R32G32B32_SFLOAT;
				description.offset = offsetof(Vertex, myPosition);
				break;
			case VertexComponent::Normal:
				description.format = VK_FORMAT_R32G32B32_SFLOAT;
				description.offset = offsetof(Vertex, myNormal);
				break;
			case VertexComponent::UV:
				description.format = VK_FORMAT_R32G32_SFLOAT;
				description.offset = offsetof(Vertex, myUV);
				break;
			case VertexComponent::Color:
				description.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				description.offset = offsetof(Vertex, myColor);
				break;
			}
			return description;
		}

		static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(const std::vector<VertexComponent>& someComponents)
		{
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
			for (uint32_t i = 0; i < (uint32_t)someComponents.size(); ++i)
			{
				attributeDescriptions.push_back(GetAttributeDescription(someComponents[i], i));
			}
			return attributeDescriptions;
		}
	};

	struct Primitive
	{
		uint32_t myFirstVertex;
		uint32_t myVertexCount;
		uint32_t myFirstIndex;
		uint32_t myIndexCount;

		glm::vec3 myMinPos = glm::vec3(FLT_MAX);
		glm::vec3 myMaxPos = glm::vec3(-FLT_MAX);

		int myMaterial = -1;
	};

	struct Mesh
	{
		Mesh();
		~Mesh();

		void Load(const tinygltf::Model& aModel, const tinygltf::Mesh& aMesh, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		std::vector<Primitive> myPrimitives;

		std::string myName;

		VulkanBuffer myUniformBuffer;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
		struct UniformBlock {
			glm::mat4 myMatrix = glm::mat4(1.0f);
		} myUniformData;
	};
}
}
