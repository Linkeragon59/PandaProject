#pragma once

#include "VulkanDeferredPipeline.h"

namespace Render
{
namespace Vulkan
{
namespace glTF
{
	struct Primitive
	{
		uint myFirstVertex = 0;
		uint myVertexCount = 0;
		uint myFirstIndex = 0;
		uint myIndexCount = 0;

		int myMaterial = -1;

		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};

	struct Mesh
	{
		typedef DeferredPipeline::Vertex Vertex;

		void Load(const tinygltf::Model& aModel, uint aMeshIndex, std::vector<Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		std::string myName;
		std::vector<Primitive> myPrimitives;
	};
}
}
}
