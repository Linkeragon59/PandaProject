#pragma once

#include "Render_VulkanBuffer.h"

namespace Render
{
	struct glTFMaterial
	{
		void Load(const tinygltf::Model& aModel, uint aMaterialIndex);
		void LoadEmpty();
		void Load();

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

		VulkanBufferPtr mySSBO;
	};
}
