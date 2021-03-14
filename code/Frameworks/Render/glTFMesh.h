#pragma once

#include "glTFVulkanPSO.h"

namespace Render
{
namespace glTF
{
	struct Primitive
	{
		uint32_t myFirstVertex = 0;
		uint32_t myVertexCount = 0;
		uint32_t myFirstIndex = 0;
		uint32_t myIndexCount = 0;

		int myMaterial = -1;
	};

	struct Mesh
	{
		void Load(const tinygltf::Model& aModel, uint32_t aMeshIndex, std::vector<VulkanPSO::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		std::string myName;

		std::vector<Primitive> myPrimitives;
	};
}
}
