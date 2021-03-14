#pragma once

#include "VulkanBuffer.h"

namespace Render
{
namespace glTF
{
	class Model;
	struct Node;

	struct Skin
	{
		~Skin();

		void Load(Model* aContainer, const tinygltf::Model& aModel, uint32_t aSkinIndex);

		void SetupDescriptorSet(VkDescriptorPool aDescriptorPool);

		std::string myName;

		Node* mySkeletonRoot = nullptr;
		std::vector<Node*> myJoints;
		std::vector<glm::mat4> myInverseBindMatrices;

		VulkanBuffer mySSBO;
		VkDescriptorSet myDescriptorSet;
	};
}
}
