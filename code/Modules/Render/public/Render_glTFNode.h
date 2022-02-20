#pragma once

#include "Render_glTFMesh.h"

#include "Render_VulkanBuffer.h"
#include "Render_ShaderHelpers.h"

namespace Render
{
	class glTFModel;

	struct glTFNode
	{
		~glTFNode();

		void Load(const tinygltf::Model& aModel, uint aNodeIndex, std::vector<glTFMesh::Vertex>& someOutVertices, std::vector<uint>& someOutIndices);

		void UpdateUBO(const glm::mat4& aMatrix);
		void UpdateJoints(const glTFModel* aContainer);
		void Draw(const glTFModel* aContainer, VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, ShaderHelpers::BindType aType);

		glm::mat4 GetLocalMatrix() const;
		glm::mat4 GetMatrix() const;

		uint myIndex = 0;
		std::string myName;

		glTFNode* myParent = nullptr;
		std::vector<glTFNode*> myChildren;

		glTFMesh myMesh;
		int mySkinIndex = -1;

		glm::vec3 myTranslation{};
		glm::quat myRotation{};
		glm::vec3 myScale{ 1.0f };
		glm::mat4 myMatrix = glm::mat4(1.0f);
		
		VulkanBufferPtr myUBO;
	};
}
