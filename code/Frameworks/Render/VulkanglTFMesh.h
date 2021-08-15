#pragma once

#include "VulkanShaderHelpers.h"

namespace Render::Vulkan::glTF
{
	struct Primitive
	{
		uint myFirstVertex = 0;
		uint myVertexCount = 0;
		uint myFirstIndex = 0;
		uint myIndexCount = 0;

		int myMaterial = -1;
	};

	struct Mesh
	{
		typedef ShaderHelpers::Vertex Vertex;

		void Load(const tinygltf::Model& aModel, uint aMeshIndex, std::vector<Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		std::string myName;
		std::vector<Primitive> myPrimitives;
	};
}
