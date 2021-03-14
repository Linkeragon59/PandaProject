#pragma once

#include "glTFAnimation.h"
#include "glTFMaterial.h"
#include "glTFNode.h"
#include "glTFSkin.h"
#include "glTFTexture.h"

#include "glTFVulkanPSO.h"

#include "VulkanBuffer.h"

namespace Render
{
namespace glTF
{
	class Model
	{
	public:
		Model();
		~Model();

		bool LoadFromFile(std::string aFilename, VkQueue aTransferQueue, float aScale = 1.0f);
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout);

		void Update();

		Node* GetNodeByIndex(uint32_t anIndex);
		const Image* GetImage(uint32_t anIndex) { return &myImages[anIndex]; }
		const Texture* GetTexture(uint32_t anIndex) { return &myTextures[anIndex]; }
		const Material* GetMaterial(uint32_t anIndex) { return &myMaterials[anIndex]; }
		const Skin* GetSkin(uint32_t anIndex) { return &mySkins[anIndex]; }

	private:
		void LoadImages(const tinygltf::Model& aModel);
		void LoadTextures(const tinygltf::Model& aModel);
		void LoadMaterials(tinygltf::Model& aModel);
		void LoadSkins(const tinygltf::Model& aModel);
		void LoadAnimations(const tinygltf::Model& aModel);

		void LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<VulkanPSO::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		template<typename Functor>
		void IterateNodes(Functor aFunctor, bool aParentFirst = true)
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

		void SetupDescriptorPool();
		void SetupDescriptorSets();

		VkDevice myDevice = VK_NULL_HANDLE;
		VkQueue myTransferQueue = VK_NULL_HANDLE;

		VulkanBuffer myVertexBuffer;
		VulkanBuffer myIndexBuffer;

		// Each model has its descriptor pool
		VkDescriptorPool myDescriptorPool = VK_NULL_HANDLE;

		uint32_t myNodeCount = 0;
		std::vector<Node*> myNodes;

		std::vector<Image> myImages;
		std::vector<Texture> myTextures;
		std::vector<Material> myMaterials;
		std::vector<Skin> mySkins;
		std::vector<Animation> myAnimations;

		uint32_t myActiveAnimation = 0;
	};
}
}
