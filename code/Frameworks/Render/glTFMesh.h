#pragma once

#include "VulkanPSOContainer.h"

namespace Render
{
	struct glTFPrimitive
	{
		uint32_t myFirstVertex = 0;
		uint32_t myVertexCount = 0;
		uint32_t myFirstIndex = 0;
		uint32_t myIndexCount = 0;

		int myMaterial = -1;

		void SetupDescriptor(VkDescriptorPool aDescriptorPool, const VkDescriptorBufferInfo* aUBODescriptor, const VkDescriptorImageInfo* aTextureDescriptor);
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};

	struct glTFMesh
	{
		glTFMesh();
		~glTFMesh();

		void Load(const tinygltf::Model& aModel, const tinygltf::Mesh& aMesh, std::vector<VulkanPSOContainer::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		std::string myName;

		std::vector<glTFPrimitive*> myPrimitives;
	};
}
