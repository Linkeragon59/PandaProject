#pragma once

#include "Render_glTFAnimation.h"
#include "Render_glTFMaterial.h"
#include "Render_glTFNode.h"
#include "Render_glTFSkin.h"
#include "Render_glTFTexture.h"

#include "Render_Model.h"
#include "Render_VulkanBuffer.h"

namespace Render
{
	class glTFModel : public Model
	{
	public:
		glTFModel(const glTFModelData& someData);
		~glTFModel();

		void Update(const ModelData& someData) override;
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType) override;

		glTFNode* GetNodeByIndex(uint anIndex);
		const glTFImage* GetImage(uint anIndex) const { return &myImages[anIndex]; }
		const glTFImage* GetEmptyImage() const { return &myImages.back(); }
		const glTFTexture* GetTexture(uint anIndex) const { return &myTextures[anIndex]; }
		const glTFMaterial* GetMaterial(uint anIndex) const { return &myMaterials[anIndex]; }
		const glTFMaterial* GetEmptyMaterial() const { return &myMaterials.back(); }
		const glTFSkin* GetSkin(uint anIndex) const { return &mySkins[anIndex]; }
		const glTFSkin* GetEmptySkin() const { return &mySkins.back(); }

	private:
		bool LoadFromFile(const std::string& aFilename, VkQueue aTransferQueue, const ModelData& someData);

		void LoadImages(const tinygltf::Model& aModel);
		void LoadTextures(const tinygltf::Model& aModel);
		void LoadMaterials(tinygltf::Model& aModel);
		void LoadSkins(const tinygltf::Model& aModel);
		void LoadAnimations(const tinygltf::Model& aModel);

		void LoadNodes(const tinygltf::Model& aModel, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

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

		VulkanBufferPtr myVertexBuffer;
		VulkanBufferPtr myIndexBuffer;

		uint myNodeCount = 0;
		std::vector<glTFNode*> myNodes;

		std::vector<glTFImage> myImages;
		std::vector<glTFTexture> myTextures;
		std::vector<glTFMaterial> myMaterials;
		std::vector<glTFSkin> mySkins;
		std::vector<glTFAnimation> myAnimations;

		uint myActiveAnimation = 0;
	};
}
