#pragma once

#include "VulkanHelpers.h"
#include "VulkanglTFTexture.h"
#include "VulkanglTFNode.h"

#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace Render
{
namespace VulkanglTF
{
	enum FileLoadingFlags
	{
		None = 0x00,
		PreTransformVertices = 0x01,
		PreMultiplyVertexColors = 0x02,
		FlipY = 0x04
	};

	enum class VertexComponent
	{
		Position,
		Normal,
		UV,
		Color
	};

	struct Vertex {
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

	class Model
	{
	public:
		Model();
		~Model();

		bool LoadFromFile(std::string aFilename, VkQueue aTransferQueue, uint32_t someLoadingFlags = FileLoadingFlags::None, float aScale = 1.0f);

	private:
		void LoadTextures(const tinygltf::Model& aModel);
		Node* LoadNode(const tinygltf::Model& aModel, uint32_t aNodeIndex, float aScale, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		VkDevice myDevice = VK_NULL_HANDLE;
		VkQueue myTransferQueue = VK_NULL_HANDLE;

		std::vector<Texture> myTextures;
		std::vector<Node*> myNodes;
	};
}
}
