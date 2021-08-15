#pragma once

#include "VulkanglTFAnimation.h"
#include "VulkanglTFMaterial.h"
#include "VulkanglTFNode.h"
#include "VulkanglTFSkin.h"
#include "VulkanglTFTexture.h"

#include "VulkanBuffer.h"
#include "VulkanModel.h"

namespace Render::Vulkan::glTF
{
	class Model : public Render::Vulkan::Model
	{
	public:
		Model(const ModelData& someData);
		~Model();

		void Update(const ModelData& someData) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) override;

		Node* GetNodeByIndex(uint anIndex);
		const Image* GetImage(uint anIndex) const { return &myImages[anIndex]; }
		const Image* GetEmptyImage() const { return &myImages.back(); }
		const Texture* GetTexture(uint anIndex) const { return &myTextures[anIndex]; }
		const Material* GetMaterial(uint anIndex) const { return &myMaterials[anIndex]; }
		const Material* GetEmptyMaterial() const { return &myMaterials.back(); }
		const Skin* GetSkin(uint anIndex) const { return &mySkins[anIndex]; }
		const Skin* GetEmptySkin() const { return &mySkins.back(); }

	private:
		bool LoadFromFile(const std::string& aFilename, VkQueue aTransferQueue, const glm::mat4& aMatrix);

		void LoadImages(const tinygltf::Model& aModel);
		void LoadTextures(const tinygltf::Model& aModel);
		void LoadMaterials(tinygltf::Model& aModel);
		void LoadSkins(const tinygltf::Model& aModel);
		void LoadAnimations(const tinygltf::Model& aModel);

		void LoadNodes(const tinygltf::Model& aModel, float aScale, std::vector<Mesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

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

		VkDevice myDevice = VK_NULL_HANDLE;
		VkQueue myTransferQueue = VK_NULL_HANDLE;

		Buffer myVertexBuffer;
		Buffer myIndexBuffer;

		uint myNodeCount = 0;
		std::vector<Node*> myNodes;

		std::vector<Image> myImages;
		std::vector<Texture> myTextures;
		std::vector<Material> myMaterials;
		std::vector<Skin> mySkins;
		std::vector<Animation> myAnimations;

		uint myActiveAnimation = 0;
	};
}
