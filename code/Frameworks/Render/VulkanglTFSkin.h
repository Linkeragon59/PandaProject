#pragma once

#include "VulkanBuffer.h"

namespace Render
{
namespace Vulkan
{
namespace glTF
{
	class Model;
	struct Node;

	struct Skin
	{
		~Skin();

		void Load(Model* aContainer, const tinygltf::Model& aModel, uint aSkinIndex);
		void LoadEmpty();

		std::string myName;

		Node* mySkeletonRoot = nullptr;
		std::vector<Node*> myJoints;
		std::vector<glm::mat4> myInverseBindMatrices;

		Buffer mySSBO;
	};
}
}
}
