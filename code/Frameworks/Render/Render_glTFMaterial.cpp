#include "Render_glTFMaterial.h"

namespace Render
{
	glTFMaterial::~glTFMaterial()
	{
		mySSBO.Destroy();
	}

	void glTFMaterial::Load(const tinygltf::Model& aModel, uint aMaterialIndex)
	{
		const tinygltf::Material& gltfMaterial = aModel.materials[aMaterialIndex];

		// TODO: values and additionalValues are deprecated, change to use the correct accessors
		if (gltfMaterial.values.find("baseColorTexture") != gltfMaterial.values.end())
		{
			myBaseColorTexture = gltfMaterial.values.at("baseColorTexture").TextureIndex();
		}
		if (gltfMaterial.values.find("baseColorFactor") != gltfMaterial.values.end())
		{
			myBaseColorFactor = glm::make_vec4(gltfMaterial.values.at("baseColorFactor").ColorFactor().data());
		}

		if (gltfMaterial.values.find("metallicRoughnessTexture") != gltfMaterial.values.end())
		{
			myMetallicRoughnessTexture = gltfMaterial.values.at("metallicRoughnessTexture").TextureIndex();
		}
		if (gltfMaterial.values.find("metallicFactor") != gltfMaterial.values.end())
		{
			myMetallicFactor = static_cast<float>(gltfMaterial.values.at("metallicFactor").Factor());
		}
		if (gltfMaterial.values.find("roughnessFactor") != gltfMaterial.values.end())
		{
			myRoughnessFactor = static_cast<float>(gltfMaterial.values.at("roughnessFactor").Factor());
		}

		if (gltfMaterial.additionalValues.find("normalTexture") != gltfMaterial.additionalValues.end())
		{
			myNormalTexture = gltfMaterial.additionalValues.at("normalTexture").TextureIndex();
		}
		if (gltfMaterial.additionalValues.find("emissiveTexture") != gltfMaterial.additionalValues.end())
		{
			myEmissiveTexture = gltfMaterial.additionalValues.at("emissiveTexture").TextureIndex();
		}
		if (gltfMaterial.additionalValues.find("occlusionTexture") != gltfMaterial.additionalValues.end())
		{
			myOcclusionTexture = gltfMaterial.additionalValues.at("occlusionTexture").TextureIndex();
		}

		if (gltfMaterial.additionalValues.find("alphaMode") != gltfMaterial.additionalValues.end())
		{
			tinygltf::Parameter param = gltfMaterial.additionalValues.at("alphaMode");
			if (param.string_value == "BLEND")
				myAlphaMode = AlphaMode::ALPHAMODE_BLEND;
			else if (param.string_value == "MASK")
				myAlphaMode = AlphaMode::ALPHAMODE_MASK;
		}
		if (gltfMaterial.additionalValues.find("alphaCutoff") != gltfMaterial.additionalValues.end())
		{
			myAlphaCutoff = static_cast<float>(gltfMaterial.additionalValues.at("alphaCutoff").Factor());
		}

		Load();
	}

	void glTFMaterial::LoadEmpty()
	{
		Load();
	}

	void glTFMaterial::Load()
	{
		// Store material info in a shader storage buffer object (SSBO)
		VkDeviceSize ssboSize = sizeof(glm::vec4);
		mySSBO.Create(ssboSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mySSBO.SetupDescriptor();
		mySSBO.Map();
		memcpy(mySSBO.myMappedData, &myBaseColorFactor, ssboSize);
		mySSBO.Unmap();
	}
}
