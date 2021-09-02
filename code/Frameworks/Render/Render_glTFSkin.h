#pragma once

#include "Render_VulkanBuffer.h"

namespace Render
{
	class glTFModel;
	struct glTFNode;

	struct glTFSkin
	{
		~glTFSkin();

		void Load(glTFModel* aContainer, const tinygltf::Model& aModel, uint aSkinIndex);
		void LoadEmpty();

		std::string myName;

		glTFNode* mySkeletonRoot = nullptr;
		std::vector<glTFNode*> myJoints;
		std::vector<glm::mat4> myInverseBindMatrices;

		VulkanBuffer mySSBO;
	};
}
