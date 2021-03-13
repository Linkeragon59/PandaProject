#pragma once

#include "glTFNode.h"
#include "glTFTexture.h"
#include "glTFMaterial.h"
#include "glTFAnimation.h"

#include "VulkanBuffer.h"
#include "VulkanPSOContainer.h"

namespace Render
{
	class glTFModel
	{
	public:
		glTFModel();
		~glTFModel();

		bool LoadFromFile(std::string aFilename, VkQueue aTransferQueue, float aScale = 1.0f);
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout);

		void Update();

	private:
		void LoadTextures(const tinygltf::Model& aModel);
		void LoadMaterials(tinygltf::Model& aModel);

		void LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<VulkanPSOContainer::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);
		glTFNode* GetNodeByIndex(uint32_t anIndex);

		void LoadAnimations(const tinygltf::Model& aModel);

		template<typename Functor>
		void IterateNodes(Functor aFunctor, bool aParentFirst = true)
		{
			for (glTFNode* node : myNodes)
				IterateNodeChildren(aFunctor, node, aParentFirst);
		}
		template<typename Functor>
		void IterateNodeChildren(Functor aFunctor, glTFNode* aNode, bool aParentFirst)
		{
			if (aParentFirst)
				aFunctor(aNode);
			for (glTFNode* childNode : aNode->myChildren)
				IterateNodeChildren(aFunctor, childNode, aParentFirst);
			if (!aParentFirst)
				aFunctor(aNode);
		}

		VkDevice myDevice = VK_NULL_HANDLE;
		VkQueue myTransferQueue = VK_NULL_HANDLE;

		// Create a new descriptor pool for each model
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		// TODO: Avoid using allocated objects?
		std::vector<glTFTexture*> myTextures;
		std::vector<glTFMaterial*> myMaterials;
		uint32_t myNodeCount = 0;
		std::vector<glTFNode*> myNodes;

		std::vector<glTFAnimation> myAnimations;
		float myAnimationTime = -1.f; // Temp

		VulkanBuffer myVertexBuffer;
		VulkanBuffer myIndexBuffer;
	};
}
