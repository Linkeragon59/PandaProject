#include "VulkanglTFSkin.h"

#include "VulkanglTFModel.h"
#include "VulkanglTFNode.h"

#include "VulkanRender.h"
#include "VulkanHelpers.h"

namespace Render::Vulkan::glTF
{
	Skin::~Skin()
	{
		mySSBO.Destroy();
	}

	void Skin::Load(Model* aContainer, const tinygltf::Model& aModel, uint aSkinIndex)
	{
		const tinygltf::Skin& gltfSkin = aModel.skins[aSkinIndex];

		myName = gltfSkin.name;

		mySkeletonRoot = aContainer->GetNodeByIndex(gltfSkin.skeleton);
		for (int joint : gltfSkin.joints)
		{
			Node* jointNode = aContainer->GetNodeByIndex(joint);
			if (jointNode)
				myJoints.push_back(jointNode);
		}

		if (gltfSkin.inverseBindMatrices > -1)
		{
			const tinygltf::Accessor& accessor = aModel.accessors[gltfSkin.inverseBindMatrices];
			const tinygltf::BufferView& bufferView = aModel.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = aModel.buffers[bufferView.buffer];

			myInverseBindMatrices.resize(accessor.count);
			VkDeviceSize ssboSize = accessor.count * sizeof(glm::mat4);

			memcpy(myInverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], ssboSize);

			// Store inverse bind matrices for this skin in a shader storage buffer object (SSBO)
			mySSBO.Create(ssboSize,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			mySSBO.SetupDescriptor();
			mySSBO.Map();
			memcpy(mySSBO.myMappedData, myInverseBindMatrices.data(), ssboSize);
		}

		Assert(myJoints.size() == myInverseBindMatrices.size());
	}

	void Skin::LoadEmpty()
	{
		VkDeviceSize ssboSize = 1 * sizeof(glm::mat4);
		glm::mat4 identity(1.0f);
		mySSBO.Create(ssboSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mySSBO.SetupDescriptor();
		mySSBO.Map();
		memcpy(mySSBO.myMappedData, &identity, ssboSize);
	}
}
