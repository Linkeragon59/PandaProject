#pragma once

#include "glTFMesh.h"

#include "glTFVulkanPSO.h"

#include "VulkanBuffer.h"

namespace Render
{
namespace glTF
{
	class Model;

	struct Node
	{
		~Node();

		void Load(const tinygltf::Model& aModel, uint32_t aNodeIndex, float aScale, std::vector<VulkanPSO::Vertex>& someOutVertices, std::vector<uint32_t>& someOutIndices);

		void SetupDescriptorSet(VkDescriptorPool aDescriptorPool);

		void UpdateUBO();
		void UpdateJoints(Model* aContainer);
		void Draw(Model* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout);

		glm::mat4 GetLocalMatrix() const;
		glm::mat4 GetMatrix() const;

		uint32_t myIndex = 0;
		std::string myName;

		Node* myParent = nullptr;
		std::vector<Node*> myChildren;

		Mesh myMesh;
		int mySkinIndex = -1;

		glm::vec3 myTranslation{};
		glm::quat myRotation{};
		glm::vec3 myScale{ 1.0f };
		glm::mat4 myMatrix = glm::mat4(1.0f);
		
		VulkanBuffer myUBO;
		VkDescriptorSet myDescriptorSet = VK_NULL_HANDLE;
	};
}
}
