#pragma once

#include "glTFMesh.h"
#include "glTFTexture.h"

#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanPSOContainer.h"

namespace Render
{
	struct glTFNode
	{
		glTFNode();
		~glTFNode();

		void Load(const tinygltf::Model& aModel, uint32_t aNodeIndex, float aScale, std::vector<VulkanPSOContainer::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		void Update();

		glm::mat4 GetLocalMatrix() const;
		glm::mat4 GetMatrix() const;

		glTFNode* myParent = nullptr;
		std::vector<glTFNode*> myChildren;

		uint32_t myIndex = 0;
		std::string myName;

		glm::vec3 myTranslation{};
		glm::quat myRotation{};
		glm::vec3 myScale{ 1.0f };
		glm::mat4 myMatrix = glm::mat4(1.0f);

		glTFMesh* myMesh = nullptr;
		VulkanBuffer myUniformBuffer;
	};
}
