#pragma once

#include "VulkanHelpers.h"
#include "VulkanglTFTexture.h"
#include "VulkanglTFMaterial.h"
#include "VulkanglTFNode.h"
#include "VulkanBuffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace Render
{
namespace VulkanglTF
{
	enum class FileLoadingFlags
	{
		None = 0x00,
		PreTransformVertices = 0x01,
		PreMultiplyVertexColors = 0x02,
		FlipY = 0x04
	};

	enum class DescriptorBindingFlags
	{
		None = 0x00,
		ImageBaseColor = 0x01,
		ImageNormalMap = 0x02
	};

	class Model
	{
	public:
		Model();
		~Model();

		bool LoadFromFile(std::string aFilename, VkQueue aTransferQueue, float aScale = 1.0f,
			uint32_t someLoadingFlags = (uint32_t)FileLoadingFlags::None,
			uint32_t someBindingFlags = (uint32_t)DescriptorBindingFlags::ImageBaseColor);

	private:
		void LoadTextures(const tinygltf::Model& aModel);
		void LoadMaterials(tinygltf::Model& aModel);
		void LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		Texture* GetTexture(int anIndex);
		Material* GetMaterial(int anIndex);

		template<typename Functor>
		void IterateNodes(Functor aFunctor, bool aParentFirst)
		{
			for (Node* node : myNodes)
				IterateNodeChildren(aFunctor, node, aParentFirst);
		}
		template<typename Functor>
		void IterateNodeChildren(Functor aFunctor, Node* aNode, bool aParentFirst)
		{
			if (aParentFirst)
				aFunctor(aNode);
			for (Node* childNode : aNode->myChildren)
				IterateNodeChildren(aFunctor, childNode, aParentFirst);
			if (!aParentFirst)
				aFunctor(aNode);
		}

		VkDevice myDevice = VK_NULL_HANDLE;
		VkQueue myTransferQueue = VK_NULL_HANDLE;

		// Create a new descriptor pool for each model
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		std::vector<Texture> myTextures;
		std::vector<Material> myMaterials;
		std::vector<Node*> myNodes;

		VulkanBuffer myVertexBuffer;
		VulkanBuffer myIndexBuffer;
	};
}
}
