#include "glTFMaterial.h"

namespace Render
{
	void glTFMaterial::Load(const tinygltf::Model& aModel, tinygltf::Material& aMaterial)
	{
		if (aMaterial.values.find("baseColorTexture") != aMaterial.values.end())
		{
			myBaseColorTexture = aModel.textures[aMaterial.values["baseColorTexture"].TextureIndex()].source;
		}
		if (aMaterial.values.find("baseColorFactor") != aMaterial.values.end())
		{
			myBaseColorFactor = glm::make_vec4(aMaterial.values["baseColorFactor"].ColorFactor().data());
		}

		if (aMaterial.values.find("metallicRoughnessTexture") != aMaterial.values.end())
		{
			myMetallicRoughnessTexture = aModel.textures[aMaterial.values["metallicRoughnessTexture"].TextureIndex()].source;
		}
		if (aMaterial.values.find("metallicFactor") != aMaterial.values.end())
		{
			myMetallicFactor = static_cast<float>(aMaterial.values["metallicFactor"].Factor());
		}
		if (aMaterial.values.find("roughnessFactor") != aMaterial.values.end())
		{
			myRoughnessFactor = static_cast<float>(aMaterial.values["roughnessFactor"].Factor());
		}

		if (aMaterial.additionalValues.find("normalTexture") != aMaterial.additionalValues.end())
		{
			myNormalTexture = aModel.textures[aMaterial.additionalValues["normalTexture"].TextureIndex()].source;
		}
		if (aMaterial.additionalValues.find("emissiveTexture") != aMaterial.additionalValues.end())
		{
			myEmissiveTexture = aModel.textures[aMaterial.additionalValues["emissiveTexture"].TextureIndex()].source;
		}
		if (aMaterial.additionalValues.find("occlusionTexture") != aMaterial.additionalValues.end())
		{
			myOcclusionTexture = aModel.textures[aMaterial.additionalValues["occlusionTexture"].TextureIndex()].source;
		}

		if (aMaterial.additionalValues.find("alphaMode") != aMaterial.additionalValues.end())
		{
			tinygltf::Parameter param = aMaterial.additionalValues["alphaMode"];
			if (param.string_value == "BLEND")
				myAlphaMode = AlphaMode::ALPHAMODE_BLEND;
			else if (param.string_value == "MASK")
				myAlphaMode = AlphaMode::ALPHAMODE_MASK;
		}
		if (aMaterial.additionalValues.find("alphaCutoff") != aMaterial.additionalValues.end())
		{
			myAlphaCutoff = static_cast<float>(aMaterial.additionalValues["alphaCutoff"].Factor());
		}
	}
}
