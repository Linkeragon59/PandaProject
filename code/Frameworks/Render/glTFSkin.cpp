#include "glTFSkin.h"

#include "glTFModel.h"
#include "glTFNode.h"

#include "VulkanRenderer.h"
#include "VulkanHelpers.h"

namespace Render
{
namespace glTF
{

	Skin::~Skin()
	{
		mySSBO.Destroy();
	}

	void Skin::Load(Model* aContainer, const tinygltf::Model& aModel, uint32_t aSkinIndex)
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

	void Skin::SetupDescriptorSet(VkDescriptorPool aDescriptorPool)
	{
		VkDevice device = Render::Vulkan::Renderer::GetInstance()->GetDevice();

		std::array<VkDescriptorSetLayout, 1> layouts = { Render::Vulkan::DeferredPipeline::ourDescriptorSetLayoutPerSkin };

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.descriptorPool = aDescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = layouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = (uint32_t)layouts.size();

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &myDescriptorSet), "Failed to create the skin descriptor set");

		std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

		// Binding 0 : Vertex shader storage buffer
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = myDescriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[0].pBufferInfo = &mySSBO.myDescriptor;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}
}
