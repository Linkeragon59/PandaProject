#pragma once

namespace Render
{
namespace glTF
{
	struct Material
	{
		void Load(const tinygltf::Model& aModel, uint32_t aMaterialIndex);

		int myBaseColorTexture = -1;
		glm::vec4 myBaseColorFactor = glm::vec4(1.0f);

		int myMetallicRoughnessTexture = -1;
		float myMetallicFactor = 1.0f;
		float myRoughnessFactor = 1.0f;

		int myNormalTexture = -1;
		int myEmissiveTexture = -1;
		int myOcclusionTexture = -1;

		enum class AlphaMode
		{
			ALPHAMODE_OPAQUE,
			ALPHAMODE_MASK,
			ALPHAMODE_BLEND
		};
		AlphaMode myAlphaMode = AlphaMode::ALPHAMODE_OPAQUE;
		float myAlphaCutoff = 1.0f;


	};
}
}
