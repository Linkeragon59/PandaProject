#include "Render_glTFSkin.h"

#include "Render_glTFModel.h"
#include "Render_glTFNode.h"

namespace Render
{
	void glTFSkin::Load(glTFModel* aContainer, const tinygltf::Model& aModel, uint aSkinIndex)
	{
		const tinygltf::Skin& gltfSkin = aModel.skins[aSkinIndex];

		myName = gltfSkin.name;

		mySkeletonRoot = aContainer->GetNodeByIndex(gltfSkin.skeleton);
		for (int joint : gltfSkin.joints)
		{
			glTFNode* jointNode = aContainer->GetNodeByIndex(joint);
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
			mySSBO = new VulkanBuffer(ssboSize,
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			mySSBO->SetupDescriptor();
			mySSBO->Map();
			memcpy(mySSBO->myMappedData, myInverseBindMatrices.data(), ssboSize);
		}

		Assert(myJoints.size() == myInverseBindMatrices.size());
	}

	void glTFSkin::LoadEmpty()
	{
		VkDeviceSize ssboSize = 1 * sizeof(glm::mat4);
		glm::mat4 identity(1.0f);
		mySSBO = new VulkanBuffer(ssboSize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		mySSBO->SetupDescriptor();
		mySSBO->Map();
		memcpy(mySSBO->myMappedData, &identity, ssboSize);
	}
}
