#pragma once

#include "VulkanglTFMesh.h"

#include "VulkanBuffer.h"

namespace Render::Vulkan::glTF
{
	class Model;

	struct Node
	{
		~Node();

		void Load(const tinygltf::Model& aModel, uint aNodeIndex, float aScale, std::vector<Mesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		void SetupDescriptorSet(Model* aContainer, VkDescriptorPool aDescriptorPool);

		void UpdateUBO();
		void UpdateJoints(Model* aContainer);
		void Draw(Model* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex);

		glm::mat4 GetLocalMatrix() const;
		glm::mat4 GetMatrix() const;

		uint myIndex = 0;
		std::string myName;

		Node* myParent = nullptr;
		std::vector<Node*> myChildren;

		Mesh myMesh;
		int mySkinIndex = -1;

		glm::vec3 myTranslation{};
		glm::quat myRotation{};
		glm::vec3 myScale{ 1.0f };
		glm::mat4 myMatrix = glm::mat4(1.0f);
		
		Buffer myUBO;
	};
}
