#pragma once

#include "Render_ShaderHelpers.h"

namespace Render
{
	struct glTFPrimitive
	{
		uint myFirstVertex = 0;
		uint myVertexCount = 0;
		uint myFirstIndex = 0;
		uint myIndexCount = 0;

		int myMaterial = -1;
	};

	struct glTFMesh
	{
		typedef ShaderHelpers::Vertex Vertex;

		void Load(const tinygltf::Model& aModel, uint aMeshIndex, std::vector<Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		std::string myName;
		std::vector<glTFPrimitive> myPrimitives;
	};
}
